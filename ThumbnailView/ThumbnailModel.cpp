/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "ThumbnailModel.h"
#include <QDebug>
#include "CellGeometry.h"
#include <QPainter>
#include "ThumbnailRequest.h"
#include "DB/ImageDB.h"
#include "ThumbnailWidget.h"
#include "ImageManager/AsyncLoader.h"
#include "Settings/SettingsData.h"
#include "Utilities/Util.h"
#include "ImageManager/ThumbnailCache.h"
#include "SelectionMaintainer.h"
#include <DB/FileName.h>

ThumbnailView::ThumbnailModel::ThumbnailModel( ThumbnailFactory* factory)
    : ThumbnailComponent( factory ),
      _sortDirection( Settings::SettingsData::instance()->showNewestThumbnailFirst() ? NewestFirst : OldestFirst )
{
    connect( DB::ImageDB::instance(), SIGNAL( imagesDeleted( const DB::IdList& ) ), this, SLOT( imagesDeletedFromDB( const DB::IdList& ) ) );
}

static bool stackOrderComparator(const DB::Id& a, const DB::Id& b) {
    return a.fetchInfo()->stackOrder() < b.fetchInfo()->stackOrder();
}

void ThumbnailView::ThumbnailModel::updateDisplayModel()
{
    ImageManager::AsyncLoader::instance()->stop( model(), ImageManager::StopOnlyNonPriorityLoads );

    // Note, this can be simplified, if we make the database backend already
    // return things in the right order. Then we only need one pass while now
    // we need to go through the list two times.

    /* Extract all stacks we have first. Different stackid's might be
     * intermingled in the result so we need to know this ahead before
     * creating the display list.
     */
    typedef QList<DB::Id> StackList;
    typedef QMap<DB::StackID, StackList> StackMap;
    StackMap stackContents;
    Q_FOREACH(const DB::Id& id, ZZZ(_imageList)) {
        DB::ImageInfoPtr imageInfo = id.fetchInfo();
        if ( !imageInfo.isNull() && imageInfo->isStacked() ) {
            DB::StackID stackid = imageInfo->stackId();
            stackContents[stackid].append(id);
        }
    }

    /*
     * All stacks need to be ordered in their stack order. We don't rely that
     * the images actually came in the order necessary.
     */
    for (StackMap::iterator it = stackContents.begin(); it != stackContents.end(); ++it) {
        qStableSort(it->begin(), it->end(), stackOrderComparator);
    }

    /* Build the final list to be displayed. That is basically the sequence
     * we got from the original, but the stacks shown with all images together
     * in the right sequence or collapsed showing only the top image.
     */
    _displayList = DB::FileNameList();
    QSet<DB::StackID> alreadyShownStacks;
    Q_FOREACH( const DB::Id& id, ZZZ(_imageList)) {
        DB::ImageInfoPtr imageInfo = id.fetchInfo();
        if ( !imageInfo.isNull() && imageInfo->isStacked()) {
            DB::StackID stackid = imageInfo->stackId();
            if (alreadyShownStacks.contains(stackid))
                continue;
            StackMap::iterator found = stackContents.find(stackid);
            Q_ASSERT(found != stackContents.end());
            const StackList& orderedStack = *found;
            if (_expandedStacks.contains(stackid)) {
                Q_FOREACH( const DB::Id& id, orderedStack) {
                    _displayList.append(ZZZ(id));
                }
            } else {
                _displayList.append(ZZZ(orderedStack.at(0)));
            }
            alreadyShownStacks.insert(stackid);
        }
        else {
            _displayList.append(ZZZ(id));
        }
    }

    if ( _sortDirection != OldestFirst )
        _displayList = ZZZ(ZZZ(_displayList).reversed());

    updateIndexCache();

    emit collapseAllStacksEnabled( _expandedStacks.size() > 0);
    emit expandAllStacksEnabled( _allStacks.size() != model()->_expandedStacks.size() );
    reset();
}

void ThumbnailView::ThumbnailModel::toggleStackExpansion(const DB::Id& id)
{
    DB::ImageInfoPtr imageInfo = id.fetchInfo();
    if (imageInfo) {
        DB::StackID stackid = imageInfo->stackId();
        if (_expandedStacks.contains(stackid))
            _expandedStacks.remove(stackid);
        else
            _expandedStacks.insert(stackid);
        updateDisplayModel();
        model()->reset();
    }
}

void ThumbnailView::ThumbnailModel::collapseAllStacks()
{
    _expandedStacks.clear();
    updateDisplayModel();
}

void ThumbnailView::ThumbnailModel::expandAllStacks()
{
    _expandedStacks = _allStacks;
    updateDisplayModel();
}


void ThumbnailView::ThumbnailModel::setImageList(const DB::IdList& items)
{
    _imageList = ZZZ(items);
    _allStacks.clear();
    Q_FOREACH( const DB::ImageInfoPtr& info, items.fetchInfos()) {
        if ( info && info->isStacked() )
            _allStacks << info->stackId();
    }
    updateDisplayModel();
    preloadThumbnails();
}

// TODO(hzeller) figure out if this should return the _imageList or _displayList.
DB::IdList ThumbnailView::ThumbnailModel::imageList(Order order) const
{
    if ( order == SortedOrder &&  _sortDirection == NewestFirst )
        return ZZZ(_displayList.reversed());
    else
        return ZZZ(_displayList);
}

void ThumbnailView::ThumbnailModel::imagesDeletedFromDB( const DB::IdList& list )
{
    SelectionMaintainer dummy(widget(),model());

    Q_FOREACH( const DB::Id& id, list ) {
        _displayList.removeAll( ZZZ(id) );
        _imageList.removeAll(ZZZ(id));
    }
    updateDisplayModel();
}


int ThumbnailView::ThumbnailModel::indexOf(const DB::Id& id ) const
{
    Q_ASSERT( !id.isNull() );
    if ( !_fileNameToIndex.contains(ZZZ(id)) )
        return -1;
    else
        return _fileNameToIndex[ZZZ(id)];
}

void ThumbnailView::ThumbnailModel::updateIndexCache()
{
    _fileNameToIndex.clear();
    int index = 0;
    Q_FOREACH( const DB::Id& id, ZZZ(_displayList)) {
        _fileNameToIndex[ZZZ(id)] = index;
        ++index;
    }

}

DB::Id ThumbnailView::ThumbnailModel::rightDropItem() const
{
    return ZZZ(_rightDrop);
}

void ThumbnailView::ThumbnailModel::setRightDropItem( const DB::Id& item )
{
    _rightDrop = ZZZ(item);
}

DB::Id ThumbnailView::ThumbnailModel::leftDropItem() const
{
    return ZZZ(_leftDrop);
}

void ThumbnailView::ThumbnailModel::setLeftDropItem( const DB::Id& item )
{
    _leftDrop = ZZZ(item);
}

void ThumbnailView::ThumbnailModel::setSortDirection( SortDirection direction )
{
    if ( direction == _sortDirection )
        return;

    Settings::SettingsData::instance()->setShowNewestFirst( direction == NewestFirst );
    _displayList = _displayList.reversed();
    updateIndexCache();

    _sortDirection = direction;
}

bool ThumbnailView::ThumbnailModel::isItemInExpandedStack( const DB::StackID& id ) const
{
    return _expandedStacks.contains(id);
}

int ThumbnailView::ThumbnailModel::imageCount() const
{
    return _displayList.size();
}

void ThumbnailView::ThumbnailModel::setOverrideImage(const DB::Id &id, const QPixmap &pixmap)
{
    if ( pixmap.isNull() )
        m_overrideFileName = DB::FileName();
    else {
        m_overrideFileName = ZZZ(id);
        m_overrideImage = pixmap;
    }
    emit dataChanged( idToIndex(id), idToIndex(id) );
}

DB::Id ThumbnailView::ThumbnailModel::imageAt( int index ) const
{
    Q_ASSERT( index >= 0 && index < imageCount() );
    return ZZZ(_displayList.at(index));
}

int ThumbnailView::ThumbnailModel::rowCount(const QModelIndex&) const
{
    return imageCount();
}

QVariant ThumbnailView::ThumbnailModel::data(const QModelIndex& index, int role ) const
{
    if ( !index.isValid() || static_cast<uint>(index.row()) >= _displayList.size())
        return QVariant();

    if ( role == Qt::DecorationRole ) {
        const DB::Id mediaId = ZZZ(_displayList.at(index.row()));
        return pixmap( mediaId );
    }

    if ( role == Qt::DisplayRole )
        return thumbnailText( index );

    return QVariant();
}

void ThumbnailView::ThumbnailModel::requestThumbnail( const DB::Id& mediaId, const ImageManager::Priority priority )
{
    DB::ImageInfoPtr imageInfo = mediaId.fetchInfo();
    if ( imageInfo.isNull() )
        return;
    const QSize cellSize = cellGeometryInfo()->preferredIconSize();
    const int angle = imageInfo->angle();
    ThumbnailRequest* request
        = new ThumbnailRequest( _displayList.indexOf(ZZZ(mediaId)), imageInfo->fileName(), cellSize, angle, this );
    request->setPriority( priority );
    ImageManager::AsyncLoader::instance()->load( request );
}

void ThumbnailView::ThumbnailModel::pixmapLoaded( const DB::FileName& fileName, const QSize& size, const QSize& fullSize, int, const QImage& image, const bool loadedOK)
{
#if 0 // THIS ISN'T EVEN USED!!!!
    QPixmap pixmap( size );
    if ( loadedOK && !image.isNull() )
        pixmap = QPixmap::fromImage( image );
#endif
    DB::ImageInfoPtr imageInfo = fileName.info();
    // TODO(hzeller): figure out, why the size is set here. We do an implicit
    // write here to the database.
    if ( fullSize.isValid() && !imageInfo.isNull() ) {
        imageInfo->setSize( fullSize );
    }

    emit dataChanged( idToIndex(ZZZ(fileName)), idToIndex(ZZZ(fileName)) );
}

void ThumbnailView::ThumbnailModel::reset()
{
    QAbstractItemModel::reset();
}

int ThumbnailView::ThumbnailModel::indexOf( const DB::Id& id )
{
    return _displayList.indexOf(ZZZ(id));
}


QString ThumbnailView::ThumbnailModel::thumbnailText( const QModelIndex& index ) const
{
    const DB::Id mediaId = imageAt( index.row() );

    QString text;

    const QSize cellSize = cellGeometryInfo()->preferredIconSize();
    const int thumbnailHeight = cellSize.height() - 2 * Settings::SettingsData::instance()->thumbnailSpace();
    const int thumbnailWidth = cellSize.width(); // no subtracting here
    const int maxCharacters = thumbnailHeight / QFontMetrics( widget()->font() ).maxWidth() * 2;

    if ( Settings::SettingsData::instance()->displayLabels()) {
        QString line = mediaId.fetchInfo()->label();
        if ( QFontMetrics( widget()->font() ).width( line ) > thumbnailWidth ) {
            line = line.left( maxCharacters );
            line += QString::fromLatin1( " ..." );
        }
        text += line + QString::fromLatin1("\n");
    }

    if ( Settings::SettingsData::instance()->displayCategories()) {
        QStringList grps = mediaId.fetchInfo()->availableCategories();
        for( QStringList::const_iterator it = grps.constBegin(); it != grps.constEnd(); ++it ) {
            QString category = *it;
            if ( category != QString::fromLatin1( "Folder" ) && category != QString::fromLatin1( "Media Type" ) ) {
                Utilities::StringSet items = mediaId.fetchInfo()->itemsOfCategory( category );
                if (!items.empty()) {
                    QString line;
                    bool first = true;
                    for( Utilities::StringSet::const_iterator it2 = items.begin(); it2 != items.end(); ++it2 ) {
                        QString item = *it2;
                        if ( first )
                            first = false;
                        else
                            line += QString::fromLatin1( ", " );
                        line += item;
                    }
                    if ( QFontMetrics( widget()->font() ).width( line ) > thumbnailWidth ) {
                        line = line.left( maxCharacters );
                        line += QString::fromLatin1( " ..." );
                    }
                    text += line + QString::fromLatin1( "\n" );
                }
            }
        }
    }

    if(text.isEmpty())
        text = QString::fromLatin1( "" );

    return text.trimmed();
}

void ThumbnailView::ThumbnailModel::updateCell( int row )
{
    updateCell( index( row, 0 ) );
}

void ThumbnailView::ThumbnailModel::updateCell( const QModelIndex& index )
{
    emit dataChanged( index, index );
}

void ThumbnailView::ThumbnailModel::updateCell( const DB::Id& id )
{
    updateCell( indexOf( id ) );
}

QModelIndex ThumbnailView::ThumbnailModel::idToIndex( const DB::Id& id ) const
{
    if ( id.isNull() )
        return QModelIndex();
    else
        return index( indexOf(id), 0 );
}

QPixmap ThumbnailView::ThumbnailModel::pixmap( const DB::Id& mediaId ) const
{
    if ( m_overrideFileName == ZZZ(mediaId))
        return m_overrideImage;

    const DB::ImageInfoPtr imageInfo = mediaId.fetchInfo();
    if (imageInfo == DB::ImageInfoPtr(NULL) )
        return QPixmap();
    const DB::FileName fileName = imageInfo->fileName();

    if ( ImageManager::ThumbnailCache::instance()->contains( fileName ) )
        return ImageManager::ThumbnailCache::instance()->lookup( fileName );

    const_cast<ThumbnailView::ThumbnailModel*>(this)->requestThumbnail( mediaId, ImageManager::ThumbnailVisible );
    return QPixmap();
}

bool ThumbnailView::ThumbnailModel::thumbnailStillNeeded( int row ) const
{
    return ( row >= _firstVisibleRow && row <= _lastVisibleRow );
}

void ThumbnailView::ThumbnailModel::updateVisibleRowInfo()
{
    _firstVisibleRow = widget()->indexAt( QPoint(0,0) ).row();
    const int columns = widget()->width() / cellGeometryInfo()->cellSize().width();
    const int rows = widget()->height() / cellGeometryInfo()->cellSize().height();
    _lastVisibleRow = qMin(_firstVisibleRow + columns*(rows+1), rowCount(QModelIndex()));
}

void ThumbnailView::ThumbnailModel::preloadThumbnails()
{
    // FIXME: it would make a lot of sense to merge preloadThumbnails() with pixmap()
    // and maybe also move the caching stuff into the ImageManager
    Q_FOREACH( const DB::Id item, ZZZ(_displayList)) {
        const DB::ImageInfoPtr imageInfo = item.fetchInfo();
        if ( imageInfo.isNull() )
            continue;
        const DB::FileName fileName = imageInfo->fileName();

        if ( ImageManager::ThumbnailCache::instance()->contains( fileName ) )
            continue;
        const_cast<ThumbnailView::ThumbnailModel*>(this)->requestThumbnail( item, ImageManager::ThumbnailInvisible );
    }
}

