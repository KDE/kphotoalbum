/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

#include <QIcon>
#include <QLoggingCategory>

#include <KLocalizedString>

#include <DB/FileName.h>
#include <DB/ImageDB.h>
#include <ImageManager/AsyncLoader.h>
#include <ImageManager/ThumbnailCache.h>
#include <Settings/SettingsData.h>
#include <Utilities/FileUtil.h>

#include "CellGeometry.h"
#include "Logging.h"
#include "ThumbnailRequest.h"
#include "ThumbnailWidget.h"
#include "SelectionMaintainer.h"

ThumbnailView::ThumbnailModel::ThumbnailModel( ThumbnailFactory* factory)
    : ThumbnailComponent( factory )
    , m_sortDirection( Settings::SettingsData::instance()->showNewestThumbnailFirst() ? NewestFirst : OldestFirst )
    , m_firstVisibleRow(-1)
    , m_lastVisibleRow(-1)
{
    connect( DB::ImageDB::instance(), SIGNAL(imagesDeleted(DB::FileNameList)), this, SLOT(imagesDeletedFromDB(DB::FileNameList)) );
    m_ImagePlaceholder = QIcon::fromTheme( QLatin1String("image-x-generic") ).pixmap( cellGeometryInfo()->preferredIconSize() );
    m_VideoPlaceholder = QIcon::fromTheme( QLatin1String("video-x-generic") ).pixmap( cellGeometryInfo()->preferredIconSize() );
    m_filter.setSearchMode(0);
    connect( this, &ThumbnailModel::filterChanged, this, &ThumbnailModel::updateDisplayModel);
}

static bool stackOrderComparator(const DB::FileName& a, const DB::FileName& b) {
    return a.info()->stackOrder() < b.info()->stackOrder();
}

void ThumbnailView::ThumbnailModel::updateDisplayModel()
{
    beginResetModel();
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
    Q_FOREACH(const DB::FileName& fileName, m_imageList) {
        DB::ImageInfoPtr imageInfo = fileName.info();
        if ( imageInfo && imageInfo->isStacked() ) {
            DB::StackID stackid = imageInfo->stackId();
            stackContents[stackid].append(fileName);
        }
    }

    /*
     * All stacks need to be ordered in their stack order. We don't rely that
     * the images actually came in the order necessary.
     */
    for (StackMap::iterator it = stackContents.begin(); it != stackContents.end(); ++it) {
        std::stable_sort(it->begin(), it->end(), stackOrderComparator);
    }

    /* Build the final list to be displayed. That is basically the sequence
     * we got from the original, but the stacks shown with all images together
     * in the right sequence or collapsed showing only the top image.
     */
    m_displayList = DB::FileNameList();
    QSet<DB::StackID> alreadyShownStacks;
    Q_FOREACH( const DB::FileName& fileName, m_imageList) {
        DB::ImageInfoPtr imageInfo = fileName.info();
        if (!m_filter.match(imageInfo))
            continue;
        if ( imageInfo && imageInfo->isStacked()) {
            DB::StackID stackid = imageInfo->stackId();
            if (alreadyShownStacks.contains(stackid))
                continue;
            StackMap::iterator found = stackContents.find(stackid);
            Q_ASSERT(found != stackContents.end());
            const StackList& orderedStack = *found;
            if (m_expandedStacks.contains(stackid)) {
                Q_FOREACH( const DB::FileName& fileName, orderedStack) {
                    m_displayList.append(fileName);
                }
            } else {
                m_displayList.append(orderedStack.at(0));
            }
            alreadyShownStacks.insert(stackid);
        }
        else {
            m_displayList.append(fileName);
        }
    }

    if ( m_sortDirection != OldestFirst )
        m_displayList = m_displayList.reversed();

    updateIndexCache();

    emit collapseAllStacksEnabled( m_expandedStacks.size() > 0);
    emit expandAllStacksEnabled( m_allStacks.size() != model()->m_expandedStacks.size() );
    endResetModel();
}

void ThumbnailView::ThumbnailModel::toggleStackExpansion(const DB::FileName& fileName)
{
    DB::ImageInfoPtr imageInfo = fileName.info();
    if (imageInfo) {
        DB::StackID stackid = imageInfo->stackId();
        model()->beginResetModel();
        if (m_expandedStacks.contains(stackid))
            m_expandedStacks.remove(stackid);
        else
            m_expandedStacks.insert(stackid);
        updateDisplayModel();
        model()->endResetModel();
    }
}

void ThumbnailView::ThumbnailModel::collapseAllStacks()
{
    m_expandedStacks.clear();
    updateDisplayModel();
}

void ThumbnailView::ThumbnailModel::expandAllStacks()
{
    m_expandedStacks = m_allStacks;
    updateDisplayModel();
}


void ThumbnailView::ThumbnailModel::setImageList(const DB::FileNameList& items)
{
    m_imageList = items;
    m_allStacks.clear();
    Q_FOREACH( const DB::FileName& fileName, items) {
        DB::ImageInfoPtr info = fileName.info();
        if ( info && info->isStacked() )
            m_allStacks << info->stackId();
    }
    updateDisplayModel();
    preloadThumbnails();
}

// TODO(hzeller) figure out if this should return the m_imageList or m_displayList.
DB::FileNameList ThumbnailView::ThumbnailModel::imageList(Order order) const
{
    if ( order == SortedOrder &&  m_sortDirection == NewestFirst )
        return m_displayList.reversed();
    else
        return m_displayList;
}

void ThumbnailView::ThumbnailModel::imagesDeletedFromDB( const DB::FileNameList& list )
{
    SelectionMaintainer dummy(widget(),model());

    Q_FOREACH( const DB::FileName& fileName, list ) {
        m_displayList.removeAll(fileName);
        m_imageList.removeAll(fileName);
    }
    updateDisplayModel();
}


int ThumbnailView::ThumbnailModel::indexOf(const DB::FileName& fileName)
{
    Q_ASSERT( !fileName.isNull() );
    if ( !m_fileNameToIndex.contains(fileName) )
        m_fileNameToIndex.insert(fileName, m_displayList.indexOf(fileName));

    return m_fileNameToIndex[fileName];
}

int ThumbnailView::ThumbnailModel::indexOf(const DB::FileName& fileName) const
{
    Q_ASSERT( !fileName.isNull() );
    if ( !m_fileNameToIndex.contains(fileName) )
        return -1;

    return m_fileNameToIndex[fileName];
}

void ThumbnailView::ThumbnailModel::updateIndexCache()
{
    m_fileNameToIndex.clear();
    int index = 0;
    Q_FOREACH( const DB::FileName& fileName, m_displayList) {
        m_fileNameToIndex[fileName] = index;
        ++index;
    }

}

DB::FileName ThumbnailView::ThumbnailModel::rightDropItem() const
{
    return m_rightDrop;
}

void ThumbnailView::ThumbnailModel::setRightDropItem( const DB::FileName& item )
{
    m_rightDrop = item;
}

DB::FileName ThumbnailView::ThumbnailModel::leftDropItem() const
{
    return m_leftDrop;
}

void ThumbnailView::ThumbnailModel::setLeftDropItem( const DB::FileName& item )
{
    m_leftDrop = item;
}

void ThumbnailView::ThumbnailModel::setSortDirection( SortDirection direction )
{
    if ( direction == m_sortDirection )
        return;

    Settings::SettingsData::instance()->setShowNewestFirst( direction == NewestFirst );
    m_displayList = m_displayList.reversed();
    updateIndexCache();

    m_sortDirection = direction;
}

bool ThumbnailView::ThumbnailModel::isItemInExpandedStack( const DB::StackID& id ) const
{
    return m_expandedStacks.contains(id);
}

int ThumbnailView::ThumbnailModel::imageCount() const
{
    return m_displayList.size();
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
    return m_displayList.at(index);
}

int ThumbnailView::ThumbnailModel::rowCount(const QModelIndex&) const
{
    return imageCount();
}

QVariant ThumbnailView::ThumbnailModel::data(const QModelIndex& index, int role ) const
{
    if ( !index.isValid() || index.row() >= m_displayList.size())
        return QVariant();

    if ( role == Qt::DecorationRole ) {
        const DB::FileName fileName = m_displayList.at(index.row());
        return pixmap( fileName );
    }

    if ( role == Qt::DisplayRole )
        return thumbnailText( index );

    return QVariant();
}

void ThumbnailView::ThumbnailModel::requestThumbnail( const DB::FileName& fileName, const ImageManager::Priority priority )
{
    DB::ImageInfoPtr imageInfo = fileName.info();
    if ( !imageInfo )
        return;
    // request the thumbnail in the size that is set in the settings, not in the current grid size:
    const QSize cellSize = cellGeometryInfo()->baseIconSize();
    const int angle = imageInfo->angle();
    const int row = indexOf(fileName);
    ThumbnailRequest* request
        = new ThumbnailRequest( row, fileName, cellSize, angle, this );
    request->setPriority( priority );
    ImageManager::AsyncLoader::instance()->load( request );
}

void ThumbnailView::ThumbnailModel::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& /*image*/)
{
    const DB::FileName fileName = request->databaseFileName();
    const QSize fullSize = request->fullSize();

    // As a result of the image being loaded, we emit the dataChanged signal, which in turn asks the delegate to paint the cell
    // The delegate now fetches the newly loaded image from the cache.

    DB::ImageInfoPtr imageInfo = fileName.info();
    // TODO(hzeller): figure out, why the size is set here. We do an implicit
    // write here to the database.
    if ( fullSize.isValid() && imageInfo ) {
        imageInfo->setSize( fullSize );
    }

    emit dataChanged(fileNameToIndex(fileName), fileNameToIndex(fileName));
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
            if ( category != i18n( "Folder" ) && category != i18n( "Media Type" ) ) {
                Utilities::StringSet items = fileName.info()->itemsOfCategory( category );

                if (Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured()
                    && ! Settings::SettingsData::instance()->untaggedImagesTagVisible()) {

                    if (category == Settings::SettingsData::instance()->untaggedCategory()) {
                        if (items.contains(Settings::SettingsData::instance()->untaggedTag())) {
                            items.remove(Settings::SettingsData::instance()->untaggedTag());
                        }
                    }
                }

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
    if (imageInfo == DB::ImageInfoPtr(nullptr) )
        return QPixmap();

    if ( ImageManager::ThumbnailCache::instance()->contains( fileName ) ) {
        // the cached thumbnail needs to be scaled to the actual thumbnail size:
        return ImageManager::ThumbnailCache::instance()->lookup( fileName ).scaled( cellGeometryInfo()->preferredIconSize(), Qt::KeepAspectRatio );
    }

    const_cast<ThumbnailView::ThumbnailModel*>(this)->requestThumbnail( fileName, ImageManager::ThumbnailVisible );
    if ( imageInfo->isVideo() )
        return m_VideoPlaceholder;
    else
        return m_ImagePlaceholder;
}

bool ThumbnailView::ThumbnailModel::isFiltered() const
{
    return !m_filter.isNull();
}

bool ThumbnailView::ThumbnailModel::thumbnailStillNeeded( int row ) const
{
    return ( row >= m_firstVisibleRow && row <= m_lastVisibleRow );
}

void ThumbnailView::ThumbnailModel::updateVisibleRowInfo()
{
    m_firstVisibleRow = widget()->indexAt( QPoint(0,0) ).row();
    const int columns = widget()->width() / cellGeometryInfo()->cellSize().width();
    const int rows = widget()->height() / cellGeometryInfo()->cellSize().height();
    m_lastVisibleRow = qMin(m_firstVisibleRow + columns*(rows+1), rowCount(QModelIndex()));

    // the cellGeometry has changed -> update placeholders
    m_ImagePlaceholder = QIcon::fromTheme( QLatin1String("image-x-generic") ).pixmap( cellGeometryInfo()->preferredIconSize() );
    m_VideoPlaceholder = QIcon::fromTheme( QLatin1String("video-x-generic") ).pixmap( cellGeometryInfo()->preferredIconSize() );
}

void ThumbnailView::ThumbnailModel::clearFilter()
{
    if (!m_filter.isNull())
    {
        qCDebug(ThumbnailViewLog) << "Filter cleared.";
        m_filter = DB::ImageSearchInfo();
        emit filterChanged();
    }
}

void ThumbnailView::ThumbnailModel::filterByRating(short rating)
{
    qCDebug(ThumbnailViewLog) << "Filter set: rating(" << rating << ")";
    m_filter.setRating(rating);
    emit filterChanged();
}

void ThumbnailView::ThumbnailModel::toggleRatingFilter(short rating)
{
    if (m_filter.rating() == rating)
    {
        filterByRating(rating);
    } else {
        filterByRating(-1);
    }
}

void ThumbnailView::ThumbnailModel::filterByCategory(const QString &category, const QString &tag)
{
    qCDebug(ThumbnailViewLog) << "Filter added: category(" << category << "," << tag << ")";

    m_filter.addAnd(category, tag);
    emit filterChanged();
}

void ThumbnailView::ThumbnailModel::toggleCategoryFilter(const QString &category, const QString &tag)
{
    auto tags = m_filter.categoryMatchText(category).split(QString::fromLatin1("&"),QString::SkipEmptyParts);
    if (tags.contains(tag))
    {
        qCDebug(ThumbnailViewLog) << "Filter removed: category(" << category << "," << tag << ")";
        tags.removeOne(tag);
        m_filter.setCategoryMatchText(category,tags.join(QString::fromLatin1(" & ")));
    } else {
        filterByCategory(category, tag);
    }
}

void ThumbnailView::ThumbnailModel::preloadThumbnails()
{
    // FIXME: it would make a lot of sense to merge preloadThumbnails() with pixmap()
    // and maybe also move the caching stuff into the ImageManager
    Q_FOREACH( const DB::FileName& fileName, m_displayList) {
        if ( fileName.isNull() )
            continue;

        if ( ImageManager::ThumbnailCache::instance()->contains( fileName ) )
            continue;
        const_cast<ThumbnailView::ThumbnailModel*>(this)->requestThumbnail( fileName, ImageManager::ThumbnailInvisible );
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
