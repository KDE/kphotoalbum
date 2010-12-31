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
#include "ImageManager/ImageClient.h"
#include "ImageManager/ImageRequest.h"
#include <QAbstractListModel>
#include "ThumbnailComponent.h"
#include "DB/Id.h"
#include "ThumbnailView/enums.h"
#include "DB/ImageInfo.h"
#include "enums.h"

namespace ThumbnailView
{
class ThumbnailFactory;

class ThumbnailModel :public QAbstractListModel, public ImageManager::ImageClient, private ThumbnailComponent
{
    Q_OBJECT

public:
    ThumbnailModel( ThumbnailFactory* factory );

    // -------------------------------------------------- QAbstractListModel
    OVERRIDE int rowCount(const QModelIndex&) const;
    OVERRIDE QVariant data(const QModelIndex&, int) const;
    void reset();
    QString thumbnailText( const QModelIndex& index ) const;
    void updateCell( int row );
    void updateCell( const QModelIndex& index );
    void updateCell( const DB::Id& id );

    // -------------------------------------------------- ImageClient API
    OVERRIDE void pixmapLoaded( const QString&, const QSize& size, const QSize& fullSize, int, const QImage&, const bool loadedOK);
    bool thumbnailStillNeeded( int row ) const;


    //-------------------------------------------------- Drag and Drop of items
    DB::Id rightDropItem() const;
    void setRightDropItem( const DB::Id& item );
    DB::Id leftDropItem() const;
    void setLeftDropItem( const DB::Id& item );

    //-------------------------------------------------- Stack
    void toggleStackExpansion(const DB::Id& id);
    void collapseAllStacks();
    void expandAllStacks();
    bool isItemInExpandedStack( const DB::StackID& id ) const;

    //-------------------------------------------------- Position Information
    DB::Id imageAt( int index ) const;
    int indexOf(const DB::Id& id ) const;
    int indexOf( const DB::Id& id );
    QModelIndex idToIndex( const DB::Id& id ) const;

    //-------------------------------------------------- Images
    void setImageList(const DB::IdList& list);
    DB::IdList imageList(Order) const;
    int imageCount() const;

    //-------------------------------------------------- Misc.
    void updateDisplayModel();
    void updateIndexCache();
    void setSortDirection( SortDirection );
    QPixmap pixmap( const DB::Id& id ) const;

public slots:
    void updateVisibleRowInfo();

signals:
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);
    void selectionChanged(int numberOfItemsSelected);


private: // Methods
    void requestThumbnail( const DB::Id& mediaId, const ImageManager::Priority priority );
    void preloadThumbnails();

private slots:
    void imagesDeletedFromDB( const DB::IdList& );


private: // Instance variables.
    /**
     * The list of images shown. The difference between _imageList and
     * _displayList is that _imageList contains all the images given to us,
     * while _displayList only includes those that currently should be
     * shown, ie. it exclude images from stacks that are collapsed and thus
     * not visible.
     */
    DB::IdList _displayList;

    /** The input list for images. See documentation for _displayList */
    DB::IdList _imageList;

    /**
     * File which should have drop indication point drawn on its left side
     */
    DB::Id _leftDrop;

    /**
     * File which should have drop indication point drawn on its right side
     */
    DB::Id _rightDrop;

    SortDirection _sortDirection;

    /**
     * All the stacks that should be shown expanded
     */
    QSet<DB::StackID> _expandedStacks;

    /** @short Store stack IDs for all images in current list
     *
     * Used by expandAllStacks. */
    QSet<DB::StackID> _allStacks;

    /**
     * A map mapping from Id to its index in _displayList.
     */
    QMap<DB::Id,int> _idToIndex;

    int _firstVisibleRow;
    int _lastVisibleRow;
};

}

#endif /* THUMBNAILMODEL_H */

