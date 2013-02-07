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
    connect( DB::ImageDB::instance(), SIGNAL( imagesDeleted( const DB::FileNameList& ) ), this, SLOT( imagesDeletedFromDB( const DB::FileNameList& ) ) );
}

static bool stackOrderComparator(const DB::FileName& a, const DB::FileName& b) {
    return a.info()->stackOrder() < b.info()->stackOrder();
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
    typedef QList<DB::FileName> StackList;
    typedef QMap<DB::StackID, StackList> StackMap;
    StackMap stackContents;
    Q_FOREACH(const DB::FileName& fileName, _imageList) {
        DB::ImageInfoPtr imageInfo = fileName.info();
        if ( !imageInfo.isNull() && imageInfo->isStacked() ) {
            DB::StackID stackid = imageInfo->stackId();
            stackContents[stackid].append(fileName);
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
    Q_FOREACH( const DB::FileName& fileName, _imageList) {
        DB::ImageInfoPtr imageInfo = fileName.info();
        if ( !imageInfo.isNull() && imageInfo->isStacked()) {
            DB::StackID stackid = imageInfo->stackId();
            if (alreadyShownStacks.contains(stackid))
                continue;
            StackMap::iterator found = stackContents.find(stackid);
            Q_ASSERT(found != stackContents.end());
            const StackList& orderedStack = *found;
            if (_expandedStacks.contains(stackid)) {
                Q_FOREACH( const DB::FileName& fileName, orderedStack) {
                    _displayList.append(fileName);
                }
            } else {
                _displayList.append(orderedStack.at(0));
            }
            alreadyShownStacks.insert(stackid);
        }
        else {
            _displayList.append(fileName);
        }
    }

    if ( _sortDirection != OldestFirst )
        _displayList = _displayList.reversed();

    updateIndexCache();

    emit collapseAllStacksEnabled( _expandedStacks.size() > 0);
    emit expandAllStacksEnabled( _allStacks.size() != model()->_expandedStacks.size() );
    reset();
}

void ThumbnailView::ThumbnailModel::toggleStackExpansion(const DB::FileName& fileName)
{
    DB::ImageInfoPtr imageInfo = fileName.info();
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


void ThumbnailView::ThumbnailModel::setImageList(const DB::FileNameList& items)
{
    _imageList = items;
    _allStacks.clear();
    Q_FOREACH( const DB::FileName& fileName, items) {
        DB::ImageInfoPtr info = fileName.info();
        if ( info && info->isStacked() )
            _allStacks << info->stackId();
    }
    updateDisplayModel();
    preloadThumbnails();
}

// TODO(hzeller) figure out if this should return the _imageList or _displayList.
DB::FileNameList ThumbnailView::ThumbnailModel::imageList(Order order) const
{
    if ( order == SortedOrder &&  _sortDirection == NewestFirst )
        return _displayList.reversed();
    else
        return _displayList;
}

void ThumbnailView::ThumbnailModel::imagesDeletedFromDB( const DB::FileNameList& list )
{
    SelectionMaintainer dummy(widget(),model());

    Q_FOREACH( const DB::FileName& fileName, list ) {
        _displayList.removeAll(fileName);
        _imageList.removeAll(fileName);
    }
    updateDisplayModel();
}


int ThumbnailView::ThumbnailModel::indexOf(const DB::FileName& fileName) const
{
    Q_ASSERT( !fileName.isNull() );
    if ( !_fileNameToIndex.contains(fileName))
        return -1;
    else
        return _fileNameToIndex[fileName];
}

void ThumbnailView::ThumbnailModel::updateIndexCache()
{
    _fileNameToIndex.clear();
    int index = 0;
    Q_FOREACH( const DB::FileName& fileName, _displayList) {
        _fileNameToIndex[fileName] = index;
        ++index;
    }

}

DB::FileName ThumbnailView::ThumbnailModel::rightDropItem() const
{
    return _rightDrop;
}

void ThumbnailView::ThumbnailModel::setRightDropItem( const DB::FileName& item )
{
    _rightDrop = item;
}

DB::FileName ThumbnailView::ThumbnailModel::leftDropItem() const
{
    return _leftDrop;
}

void ThumbnailView::ThumbnailModel::setLeftDropItem( const DB::FileName& item )
{
    _leftDrop = item;
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

void ThumbnailView::ThumbnailModel::setOverrideImage(const DB::FileName& fileName, const QPixmap &pixmap)
{
    if ( pixmap.isNull() )
        m_overrideFileName = DB::FileName();
    else {
        m_overrideFileName = fileName;
        m_overrideImage = pixmap;
    }
    emit dataChanged( fileNameToIndex(fileName), fileNameToIndex(fileName));
}

DB::FileName ThumbnailView::ThumbnailModel::imageAt( int index ) const
{
    Q_ASSERT( index >= 0 && index < imageCount() );
    return _displayList.at(index);
}

int ThumbnailView::ThumbnailModel::rowCount(const QModelIndex&) const
{
    return imageCount();
}

QVariant ThumbnailView::ThumbnailModel::data(const QModelIndex& index, int role ) const
{
    if ( !index.isValid() || index.row() >= _displayList.size())
        return QVariant();

    if ( role == Qt::DecorationRole ) {
        const DB::FileName fileName = _displayList.at(index.row());
        return pixmap( fileName );
    }

    if ( role == Qt::DisplayRole )
        return thumbnailText( index );

    return QVariant();
}

void ThumbnailView::ThumbnailModel::requestThumbnail( const DB::FileName& fileName, const ImageManager::Priority priority )
{
    DB::ImageInfoPtr imageInfo = fileName.info();
    if ( imageInfo.isNull() )
        return;
    const QSize cellSize = cellGeometryInfo()->preferredIconSize();
    const int angle = imageInfo->angle();
    ThumbnailRequest* request
        = new ThumbnailRequest( _displayList.indexOf(fileName), fileName, cellSize, angle, this );
    request->setPriority( priority );
    ImageManager::AsyncLoader::instance()->load( request );
}

void ThumbnailView::ThumbnailModel::pixmapLoaded( const DB::FileName& fileName, const QSize& , const QSize& fullSize, int, const QImage& , const bool )
{
    // As a result of the image being loaded, we emit the dataChanged signal, which in turn asks the delegate to paint the cell
    // The delegate now fetches the newly loaded image from the cache.

    DB::ImageInfoPtr imageInfo = fileName.info();
    // TODO(hzeller): figure out, why the size is set here. We do an implicit
    // write here to the database.
    if ( fullSize.isValid() && !imageInfo.isNull() ) {
        imageInfo->setSize( fullSize );
    }

    emit dataChanged(fileNameToIndex(fileName), fileNameToIndex(fileName));
}

void ThumbnailView::ThumbnailModel::reset()
{
    QAbstractItemModel::reset();
}

int ThumbnailView::ThumbnailModel::indexOf(const DB::FileName& fileName)
{
    return _displayList.indexOf(fileName);
}


QString ThumbnailView::ThumbnailModel::thumbnailText( const QModelIndex& index ) const
{
    const DB::FileName fileName = imageAt( index.row() );

    QString text;

    const QSize cellSize = cellGeometryInfo()->preferredIconSize();
    const int thumbnailHeight = cellSize.height() - 2 * Settings::SettingsData::instance()->thumbnailSpace();
    const int thumbnailWidth = cellSize.width(); // no subtracting here
    const int maxCharacters = thumbnailHeight / QFontMetrics( widget()->font() ).maxWidth() * 2;

    if ( Settings::SettingsData::instance()->displayLabels()) {
        QString line = fileName.info()->label();
        if ( QFontMetrics( widget()->font() ).width( line ) > thumbnailWidth ) {
            line = line.left( maxCharacters );
            line += QString::fromLatin1( " ..." );
        }
        text += line + QString::fromLatin1("\n");
    }

    if ( Settings::SettingsData::instance()->displayCategories()) {
        QStringList grps = fileName.info()->availableCategories();
        for( QStringList::const_iterator it = grps.constBegin(); it != grps.constEnd(); ++it ) {
            QString category = *it;
            if ( category != QString::fromLatin1( "Folder" ) && category != QString::fromLatin1( "Media Type" ) ) {
                Utilities::StringSet items = fileName.info()->itemsOfCategory( category );
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

void ThumbnailView::ThumbnailModel::updateCell( const DB::FileName& fileName )
{
    updateCell( indexOf(fileName) );
}

QModelIndex ThumbnailView::ThumbnailModel::fileNameToIndex( const DB::FileName& fileName ) const
{
    if ( fileName.isNull() )
        return QModelIndex();
    else
        return index( indexOf(fileName), 0 );
}

QPixmap ThumbnailView::ThumbnailModel::pixmap( const DB::FileName& fileName ) const
{
    if ( m_overrideFileName == fileName)
        return m_overrideImage;

    const DB::ImageInfoPtr imageInfo = fileName.info();
    if (imageInfo == DB::ImageInfoPtr(NULL) )
        return QPixmap();

    if ( ImageManager::ThumbnailCache::instance()->contains( fileName ) )
        return ImageManager::ThumbnailCache::instance()->lookup( fileName );

    const_cast<ThumbnailView::ThumbnailModel*>(this)->requestThumbnail( fileName, ImageManager::ThumbnailVisible );
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
    Q_FOREACH( const DB::FileName& fileName, _displayList) {
        if ( fileName.isNull() )
            continue;

        if ( ImageManager::ThumbnailCache::instance()->contains( fileName ) )
            continue;
        const_cast<ThumbnailView::ThumbnailModel*>(this)->requestThumbnail( fileName, ImageManager::ThumbnailInvisible );
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
