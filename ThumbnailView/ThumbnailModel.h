/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailComponent.h"
#include "DB/ResultId.h"
#include "ThumbnailView/enums.h"
#include "DB/ImageInfo.h"
#include "enums.h"

namespace ThumbnailView
{
class ThumbnailFactory;
class Cell;

class ThumbnailModel :public QObject, private ThumbnailComponent
{
    Q_OBJECT

public:
    ThumbnailModel( ThumbnailFactory* factory );

    //-------------------------------------------------- Selection
    void selectRange( Cell pos1, Cell pos2 );
    void select( int row, int col );
    void select( const Cell& );
    void select( const DB::ResultId& id );
    void setSelection( const IdSet& ids );

    void clearSelection();
    void toggleSelection( const DB::ResultId& id );
    void selectAll();

    DB::Result selection(bool keepSortOrderOfDatabase=false) const;
    IdSet selectionSet() const;

    bool isSelected( const DB::ResultId& ) const;

    void changeSingleSelection(const DB::ResultId& id);

    //-------------------------------------------------- Current Item
    DB::ResultId currentItem() const;
    void setCurrentItem( const DB::ResultId& id );
    void setCurrentItem( const Cell& cell );

    //-------------------------------------------------- Drag and Drop of items
    DB::ResultId rightDropItem() const;
    void setRightDropItem( const DB::ResultId& item );
    DB::ResultId leftDropItem() const;
    void setLeftDropItem( const DB::ResultId& item );

    //-------------------------------------------------- Stack
    void toggleStackExpansion(const DB::ResultId& id);
    void collapseAllStacks();
    void expandAllStacks();
    bool isItemInExpandedStack( const DB::StackID& id ) const;

    //-------------------------------------------------- Position Information
    DB::ResultId imageAt( int row, int col ) const;
    DB::ResultId imageAt( const Cell& cell ) const;
    DB::ResultId imageAt( const QPoint& coordinate, CoordinateSystem ) const;
    DB::ResultId imageAt( int index ) const;
    int indexOf(const DB::ResultId& id ) const;
    Cell positionForMediaId( const DB::ResultId& id ) const;

    //-------------------------------------------------- Images
    void setImageList(const DB::Result& list);
    DB::Result imageList(Order) const;
    int imageCount() const;

    //-------------------------------------------------- Misc.
    void updateDisplayModel();
    void updateIndexCache();
    void setSortDirection( SortDirection );

signals:
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);
    void selectionChanged(int numberOfItemsSelected );


private: // Methods
    void ensureCellsSorted( Cell& pos1, Cell& pos2 );
    void possibleEmitSelectionChanged();

private slots:
    void imagesDeletedFromDB( const DB::Result& );


private: // Instance variables.
    /**
     * The list of images shown. The difference between _imageList and
     * _displayList is that _imageList contains all the images given to us,
     * while _displayList only includes those that currently should be
     * shown, ie. it exclude images from stacks that are collapsed and thus
     * not visible.
     */
    DB::Result _displayList;

    /** The input list for images. See documentation for _displayList */
    DB::Result _imageList;

    /*
     * This set contains the files currently selected.
     */
    IdSet _selectedFiles;

    /**
     * File which should have drop indication point drawn on its left side
     */
    DB::ResultId _leftDrop;

    /**
     * File which should have drop indication point drawn on its right side
     */
    DB::ResultId _rightDrop;

    SortDirection _sortDirection;

    /**
     * This is the item currently having keyboard focus
     *
     * We need to store the file name for the current item rather than its
     * coordinates, as coordinates changes when the grid is resized.
     */
    DB::ResultId _currentItem;

    /**
     * All the stacks that should be shown expanded
     */
    QSet<DB::StackID> _expandedStacks;

    /** @short Store stack IDs for all images in current list
     *
     * Used by expandAllStacks. */
    QSet<DB::StackID> _allStacks;

    /**
     * A map mapping from ResultId to its index in _displayList.
     */
    QMap<DB::ResultId,int> _idToIndex;
};

}

#endif /* THUMBNAILMODEL_H */

