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
#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H

#include <QAbstractListModel>
#include <QPixmap>

#include <DB/FileNameList.h>
#include <DB/ImageInfo.h>
#include <DB/ImageSearchInfo.h>
#include <ImageManager/enums.h>
#include <ImageManager/ImageClientInterface.h>
#include <ThumbnailView/enums.h>
#include <ThumbnailView/ThumbnailComponent.h>

namespace ThumbnailView
{
class ThumbnailFactory;

class ThumbnailModel :public QAbstractListModel, public ImageManager::ImageClientInterface, private ThumbnailComponent
{
    Q_OBJECT

public:
    explicit ThumbnailModel( ThumbnailFactory* factory );

    // -------------------------------------------------- QAbstractListModel
    using QAbstractListModel::beginResetModel;
    using QAbstractListModel::endResetModel;
    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int) const override;
    QString thumbnailText( const QModelIndex& index ) const;
    void updateCell( int row );
    void updateCell( const QModelIndex& index );
    void updateCell( const DB::FileName& id );

    // -------------------------------------------------- ImageClient API
    void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image) override;
    bool thumbnailStillNeeded( int row ) const;


    //-------------------------------------------------- Drag and Drop of items
    DB::FileName rightDropItem() const;
    void setRightDropItem( const DB::FileName& item );
    DB::FileName leftDropItem() const;
    void setLeftDropItem( const DB::FileName& item );

    //-------------------------------------------------- Stack
    void toggleStackExpansion(const DB::FileName& id);
    void collapseAllStacks();
    void expandAllStacks();
    bool isItemInExpandedStack( const DB::StackID& id ) const;

    //-------------------------------------------------- Position Information
    DB::FileName imageAt( int index ) const;
    int indexOf(const DB::FileName& fileName ) const;
    int indexOf( const DB::FileName& fileName );
    QModelIndex fileNameToIndex( const DB::FileName& fileName ) const;

    //-------------------------------------------------- Images
    void setImageList(const DB::FileNameList& list);
    DB::FileNameList imageList(Order) const;
    int imageCount() const;
    void setOverrideImage( const DB::FileName& fileName, const QPixmap& pixmap );

    //-------------------------------------------------- Misc.
    void updateDisplayModel();
    void updateIndexCache();
    void setSortDirection( SortDirection );
    QPixmap pixmap( const DB::FileName& fileName ) const;

    /**
     * @brief isFiltered
     * @return \c true, if the filter is currently active, \c false otherwise.
     */
    bool isFiltered() const;

public slots:
    void updateVisibleRowInfo();

    /**
     * @brief clearFilter clears the filter so that all images in the current view are displayed.
     */
    void clearFilter();
    /**
     * @brief filterByRating sets the filter to only show images with the given rating.
     * @param rating a number between 0 and 10 (or -1 to disable)
     */
    void filterByRating(short rating);
    /**
     * @brief toggleRatingFilter sets the filter to only show images with the given rating,
     * if no rating filter is active. If the rating filter is already set to the given rating,
     * clear the rating filter.
     * @param rating a number between 0 and 10
     */
    void toggleRatingFilter(short rating);
    /**
     * @brief filterByCategory sets the filter to only show images with the given tag.
     * Calling this method again for the same category will overwrite the previous filter
     * for that category.
     * @param category
     * @param tag
     *
     * @see DB::ImageSearchinfo::setCategoryMatchText()
     */
    void filterByCategory(const QString &category, const QString &tag);
    /**
     * @brief toggleCategoryFilter is similar to filterByCategory(), except resets the
     * category filter if called again with the same value.
     * @param category
     * @param tag
     */
    void toggleCategoryFilter(const QString &category, const QString &tag);

signals:
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);
    void selectionChanged(int numberOfItemsSelected);
    void filterChanged();


private: // Methods
    void requestThumbnail( const DB::FileName& mediaId, const ImageManager::Priority priority );
    void preloadThumbnails();

private slots:
    void imagesDeletedFromDB( const DB::FileNameList& );


private: // Instance variables.
    /**
     * The list of images shown. The difference between m_imageList and
     * m_displayList is that m_imageList contains all the images given to us,
     * while m_displayList only includes those that currently should be
     * shown, ie. it exclude images from stacks that are collapsed and thus
     * not visible.
     */
    DB::FileNameList m_displayList;

    /** The input list for images. See documentation for m_displayList */
    DB::FileNameList m_imageList;

    /**
     * File which should have drop indication point drawn on its left side
     */
    DB::FileName m_leftDrop;

    /**
     * File which should have drop indication point drawn on its right side
     */
    DB::FileName m_rightDrop;

    SortDirection m_sortDirection;

    /**
     * All the stacks that should be shown expanded
     */
    QSet<DB::StackID> m_expandedStacks;

    /** @short Store stack IDs for all images in current list
     *
     * Used by expandAllStacks. */
    QSet<DB::StackID> m_allStacks;

    /**
     * A map mapping from Id to its index in m_displayList.
     */
    QMap<DB::FileName,int> m_fileNameToIndex;

    int m_firstVisibleRow;
    int m_lastVisibleRow;

    DB::FileName m_overrideFileName;
    QPixmap m_overrideImage;
    // placeholder pixmaps to be displayed before thumbnails are loaded:
    QPixmap m_ImagePlaceholder;
    QPixmap m_VideoPlaceholder;

    DB::ImageSearchInfo m_filter;
};

}

#endif /* THUMBNAILMODEL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
