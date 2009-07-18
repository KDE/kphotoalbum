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

private:
    /**
     * File which should have drop indication point drawn on its left side
     */
    DB::ResultId _leftDrop;

    /**
     * File which should have drop indication point drawn on its right side
     */
    DB::ResultId _rightDrop;

public:
    /** The input list for images */
    DB::Result _imageList;

private:
    SortDirection _sortDirection;
public:
    /*
     * This set contains the files currently selected.
     */
    IdSet _selectedFiles;

private:
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
public:

    /**
     * The list of images shown. We do indexed access to this _displayList that has been
     * changed from O(n) to O(1) in Qt4; so it is safe to use this data type.
     */
    DB::Result _displayList;

public:
    void updateDisplayModel();
    DB::ResultId mediaIdInCell( int row, int col ) const;
    DB::ResultId mediaIdInCell( const Cell& cell ) const;
    DB::ResultId mediaIdAtCoordinate( const QPoint& coordinate, CoordinateSystem ) const;
    void toggleStackExpansion(const DB::ResultId& id);
    void collapseAllStacks();
    void expandAllStacks();
    DB::Result selection(bool keepSortOrderOfDatabase=false) const;
    void setImageList(const DB::Result& list);
    DB::Result imageList(Order) const;

    DB::ResultId currentItem() const;
    void setCurrentItem( const DB::ResultId& id );
    void setCurrentItem( const Cell& cell );

    int indexOf(const DB::ResultId& id ) const;
    void updateIndexCache();
    Cell positionForMediaId( const DB::ResultId& id ) const;

    // Selection
    void selectAllCellsBetween( Cell pos1, Cell pos2, bool repaint = true );
    void selectCell( int row, int col, bool repaint = true );
    void selectCell( const Cell& );
    void clearSelection();
    void toggleSelection( const DB::ResultId& id );
    void selectAll();
    void changeSingleSelection(const DB::ResultId& id);
    void repaintAfterChangedSelection( const IdSet& oldSelection );

    DB::ResultId rightDropItem() const;
    void setRightDropItem( const DB::ResultId& item );

    DB::ResultId leftDropItem() const;
    void setLeftDropItem( const DB::ResultId& item );

    void setSortDirection( SortDirection );
    bool isItemInExpandedStack( const DB::StackID& id ) const;

signals:
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);
    void selectionChanged();

private:
    void ensureCellsSorted( Cell& pos1, Cell& pos2 );
    void possibleEmitSelectionChanged();

private slots:
    void imagesDeletedFromDB( const DB::Result& );

private:
    /**
     * A map mapping from ResultId to its index in _displayList.
     */
    QMap<DB::ResultId,int> _idToIndex;
};

}

#endif /* THUMBNAILMODEL_H */

