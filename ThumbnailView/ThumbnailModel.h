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
#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H
#include "ImageManager/ImageClientInterface.h"
#include "ImageManager/enums.h"
#include <QAbstractListModel>
#include "ThumbnailComponent.h"
#include "ThumbnailView/enums.h"
#include "DB/ImageInfo.h"
#include "enums.h"
#include <QPixmap>
#include <DB/FileNameList.h>

namespace ThumbnailView
{
class ThumbnailFactory;

class ThumbnailModel :public QAbstractListModel, public ImageManager::ImageClientInterface, private ThumbnailComponent
{
    Q_OBJECT

public:
    explicit ThumbnailModel( ThumbnailFactory* factory );

    // -------------------------------------------------- QAbstractListModel
    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int) const override;
    void reset();
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

public slots:
    void updateVisibleRowInfo();

signals:
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);
    void selectionChanged(int numberOfItemsSelected);


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
};

}

#endif /* THUMBNAILMODEL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
