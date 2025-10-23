// SPDX-FileCopyrightText: 2009-2014 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2019-2022 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2020 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2009-2025 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H

#include "ThumbnailComponent.h"
#include "enums.h"

#include <DB/ImageInfo.h>
#include <DB/search/ImageSearchInfo.h>
#include <ImageManager/ImageClientInterface.h>
#include <ImageManager/enums.h>
#include <kpabase/FileNameList.h>

#include <QAbstractListModel>
#include <QPixmap>

namespace ImageManager
{
class ThumbnailCache;
}

namespace ThumbnailView
{
class ThumbnailFactory;
class FilterWidget;

class ThumbnailModel : public QAbstractListModel, public ImageManager::ImageClientInterface, private ThumbnailComponent
{
    Q_OBJECT

public:
    explicit ThumbnailModel(ThumbnailFactory *factory, const ImageManager::ThumbnailCache *thumbnailCache);

    // -------------------------------------------------- QAbstractListModel
    using QAbstractListModel::beginResetModel;
    using QAbstractListModel::endResetModel;
    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &, int) const override;
    QString thumbnailText(const QModelIndex &index) const;
    void updateCell(int row);
    void updateCell(const QModelIndex &index);
    void updateCell(const DB::FileName &id);

    // -------------------------------------------------- ImageClient API
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    bool thumbnailStillNeeded(int row) const;

    //-------------------------------------------------- Drag and Drop of items
    DB::FileName rightDropItem() const;
    void setRightDropItem(const DB::FileName &item);
    DB::FileName leftDropItem() const;
    void setLeftDropItem(const DB::FileName &item);

    //-------------------------------------------------- Stack
    void toggleStackExpansion(const DB::FileName &id);
    void collapseAllStacks();
    void expandAllStacks();
    bool isItemInExpandedStack(const DB::StackID &id) const;

    //-------------------------------------------------- Position Information
    DB::FileName imageAt(int index) const;
    int indexOf(const DB::FileName &fileName) const;
    int indexOf(const DB::FileName &fileName);
    QModelIndex fileNameToIndex(const DB::FileName &fileName) const;

    //-------------------------------------------------- Images
    void setImageList(const DB::FileNameList &list);
    DB::FileNameList imageList(Order) const;
    int imageCount() const;

    /**
     * Sets a new thumbnail pixmap for the specified image file by setting
     * m_overrideFileName and m_overrideImage to override the default thumbnail
     * for the file.  If the pixmap is null, resets m_overrideFileName.
     *
     * @see pixmap()
     *
     * @return true if m_overrideFileName is set to a valid image in the view.
     */
    bool setOverrideImage(const DB::FileName &fileName, const QPixmap &pixmap);

    //-------------------------------------------------- Misc.
    void updateDisplayModel();
    void updateIndexCache();
    void setSortDirection(SortDirection);

    /**
     * @return the thumbnail pixmap for the specified file.  If the filename
     * matches m_overrideFileName, returns the override image instead.
     *
     * @see setOverrideImage()
     */
    QPixmap pixmap(const DB::FileName &fileName) const;

    /**
     * @brief isFiltered
     * @return \c true, if the filter is currently active, \c false otherwise.
     */
    bool isFiltered() const;

    FilterWidget *createFilterWidget(QWidget *parent);

public Q_SLOTS:
    void updateVisibleRowInfo();

    void toggleFilter(bool enable);
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
    /**
     * @brief filterByFreeformText filters by a plain text search.
     * To clear this filter specifically, pass an empty QString.
     * @param text
     */
    void filterByFreeformText(const QString &text);

Q_SIGNALS:
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);
    void selectionChanged(int numberOfItemsSelected);
    void filterChanged(const DB::ImageSearchInfo &filter);

private: // Methods
    void requestThumbnail(const DB::FileName &mediaId, const ImageManager::Priority priority);
    void preloadThumbnails();

private Q_SLOTS:
    void imagesDeletedFromDB(const DB::FileNameList &);

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

    int stringWidth(const QString &text) const;

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
     * A hash mapping from Id to its index in m_displayList.
     * The structure is not iterated over, so order doesn't matter.
     */
    QHash<DB::FileName, int> m_fileNameToIndex;

    int m_firstVisibleRow;
    int m_lastVisibleRow;

    DB::FileName m_overrideFileName;
    QPixmap m_overrideImage;
    // placeholder pixmaps to be displayed before thumbnails are loaded:
    QPixmap m_ImagePlaceholder;
    QPixmap m_VideoPlaceholder;

    DB::ImageSearchInfo m_filter;
    DB::ImageSearchInfo m_previousFilter;

    const ImageManager::ThumbnailCache *m_thumbnailCache;
};

}

#endif /* THUMBNAILMODEL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
