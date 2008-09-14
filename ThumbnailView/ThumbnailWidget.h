/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef THUMBNAILVIEW_THUMBNAILWIDGET_H
#define THUMBNAILVIEW_THUMBNAILWIDGET_H

#include <q3gridview.h>
#include <QMutex>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHash>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QWheelEvent>

#include "Cell.h"
#include "DB/ImageDate.h"
#include "DB/ImageInfo.h"
#include "DB/Result.h"
#include "DB/ResultId.h"
#include "GridResizeInteraction.h"
#include "ImageManager/ImageClient.h"
#include "MouseTrackingInteraction.h"
#include "SelectionInteraction.h"
#include "ThumbnailToolTip.h"
#include "ThumbnailCache.h"

class QTimer;
class QDateTime;

namespace ThumbnailView
{
enum SortDirection { NewestFirst, OldestFirst };

class ThumbnailWidget : public Q3GridView, public ImageManager::ImageClient {
    Q_OBJECT

private:
    typedef QSet<DB::ResultId> IdSet;

public:
    enum Order { ViewOrder, SortedOrder };

    ThumbnailWidget( QWidget* parent );

    void setImageList( const DB::ConstResultPtr& list );
    DB::ConstResultPtr imageList( Order ) const;

    OVERRIDE void paintCell ( QPainter * p, int row, int col );
    OVERRIDE void pixmapLoaded( const QString&, const QSize& size, const QSize& fullSize, int, const QImage&, const bool loadedOK);
    bool thumbnailStillNeeded( const QString& fileName ) const;
    DB::ConstResultPtr selection( bool keepSortOrderOfDatabase = false ) const;

    void reload( bool flushCache, bool clearSelection=true );
    DB::ResultId mediaIdUnderCursor() const;
    DB::ResultId currentItem() const;
    static ThumbnailWidget* theThumbnailView();
    void setCurrentItem( const DB::ResultId& id );
    void setSortDirection( SortDirection );

public slots:
    void gotoDate( const DB::ImageDate& date, bool includeRanges );
    void selectAll();
    void showToolTipsOnImages( bool b );
    void repaintScreen();
    void toggleStackExpansion(const DB::ResultId& id);
    void collapseAllStacks();
    void updateDisplayModel();

signals:
    void showImage( const DB::ResultId& id );
    void showSelection();
    void fileNameUnderCursorChanged( const QString& fileName );
    void currentDateChanged( const QDateTime& );
    void selectionChanged();
    void collapseAllStacksEnabled(bool enabled);

protected:
    // Painting
    void updateCell( const DB::ResultId& id );
    void updateCell( int row, int col );
    void paintCellBackground( QPainter*, int row, int col );
    void paintCellPixmap( QPainter*, int row, int col );
    QString thumbnailText( const DB::ResultId& mediaId ) const;
    void paintCellText( QPainter*, int row, int col );
    OVERRIDE void viewportPaintEvent( QPaintEvent* );
    void paintStackedIndicator( QPainter* painter, const QRect &rect, const DB::ResultId& mediaId);

    // Cell handling methods.
    DB::ResultId mediaIdInCell( int row, int col ) const;
    DB::ResultId mediaIdInCell( const Cell& cell ) const;
    DB::ResultId mediaIdAtCoordinate( const QPoint& coordinate, CoordinateSystem ) const;
    Cell positionForMediaId( const DB::ResultId& id ) const;
    Cell cellAtCoordinate( const QPoint& pos, CoordinateSystem ) const;

    enum VisibleState { FullyVisible, PartlyVisible };
    int firstVisibleRow( VisibleState ) const;
    int lastVisibleRow( VisibleState ) const;
    int numRowsPerPage() const;
    QRect iconGeometry( int row, int col ) const;
    QRect cellDimensions() const;
    int noOfCategoriesForImage( const DB::ResultId& mediaId ) const;
    int textHeight( bool reCalc ) const;
    QRect cellTextGeometry( int row, int col ) const;
    bool isFocusAtFirstCell() const;
    bool isFocusAtLastCell() const;
    Cell lastCell() const;
    bool isMouseOverStackIndicator( const QPoint& point );
    bool isGridResizing();

    // event handlers
    OVERRIDE void keyPressEvent( QKeyEvent* );
    OVERRIDE void keyReleaseEvent( QKeyEvent* );
    OVERRIDE void showEvent( QShowEvent* );
    OVERRIDE void mousePressEvent( QMouseEvent* );
    OVERRIDE void mouseMoveEvent( QMouseEvent* );
    OVERRIDE void mouseReleaseEvent( QMouseEvent* );
    OVERRIDE void mouseDoubleClickEvent ( QMouseEvent* );
    OVERRIDE void wheelEvent( QWheelEvent* );
    OVERRIDE void resizeEvent( QResizeEvent* );
    void keyboardMoveEvent( QKeyEvent* );
    OVERRIDE void dimensionChange ( int oldNumRows, int oldNumCols );

    // Selection
    void selectAllCellsBetween( Cell pos1, Cell pos2, bool repaint = true );
    void selectCell( int row, int col, bool repaint = true );
    void selectCell( const Cell& );
    void clearSelection();
    void toggleSelection( const DB::ResultId& id );
    void possibleEmitSelectionChanged();

    // Drag and drop
    OVERRIDE void contentsDragEnterEvent ( QDragEnterEvent * event );
    OVERRIDE void contentsDragMoveEvent ( QDragMoveEvent * );
    OVERRIDE void contentsDragLeaveEvent ( QDragLeaveEvent * );
    OVERRIDE void contentsDropEvent ( QDropEvent * );
    void removeDropIndications();

    // Misc
    void updateGridSize();
    bool isMovementKey( int key );
    void selectItems( const Cell& start, const Cell& end );
    void repaintAfterChangedSelection( const IdSet& oldSelection );
    void ensureCellsSorted( Cell& pos1, Cell& pos2 );
    DB::ResultPtr reverseList( const DB::ResultPtr& ) const;
    void updateCellSize();
    void updateIndexCache();
    QPoint viewportToContentsAdjusted( const QPoint& coordinate, CoordinateSystem system ) const;

    /**
     * For all filenames in the list, check if there are any missing
     * thumbnails and generate these in the background.
     */
    void generateMissingThumbnails( const DB::ConstResultPtr& items  ) const;

protected slots:
    void emitDateChange( int, int );
    void realDropEvent();
    void slotRepaint();
    void slotViewChanged( int, int );

private:
    //--- TODO(hzeller) these set of collections -> put in a ThumbnailModel.

    /** The input list for images */
    DB::ConstResultPtr _imageList;

    /**
     * The list of images shown. We do indexed access to this _displayList that has been
     * changed from O(n) to O(1) in Qt4; so it is safe to use this data type.
     */
    DB::ResultPtr _displayList;

    /**
     * A map mapping from filename to its index in _displayList.
     */
    QMap<DB::ResultId,int> _idToIndex;

    /**
     * All the stacks that should be shown expanded
     */
    QSet<DB::StackID> _expandedStacks;

    /**
     * When the user selects a date on the date bar the thumbnail view will
     * position itself accordingly. As a consequence, the thumbnail view
     * is telling the date bar which date it moved to. This is all fine
     * except for the fact that the date selected in the date bar, may be
     * for an image which is in the middle of a line, while the date
     * emitted from the thumbnail view is for the top most image in
     * the view (that is the first image on the line), which results in a
     * different cell being selected in the date bar, than what the user
     * selected.
     * Therefore we need this variable to disable the emission of the date
     * change while setting the date.
     */
    bool _isSettingDate;

    /*
     * This set contains the files currently selected.
     */
    IdSet _selectedFiles;

    /**
     * This is the item currently having keyboard focus
     *
     * We need to store the file name for the current item rather than its
     * coordinates, as coordinates changes when the grid is resized.
     */
    DB::ResultId _currentItem;

    static ThumbnailWidget* _instance;

    ThumbnailToolTip* _toolTip;

    GridResizeInteraction _gridResizeInteraction;
    bool _wheelResizing;
    SelectionInteraction _selectionInteraction;
    MouseTrackingInteraction _mouseTrackingHandler;
    MouseInteraction* _mouseHandler;
    friend class GridResizeInteraction;
    friend class SelectionInteraction;
    friend class MouseTrackingInteraction;

    /**
     * File which should have drop indication point drawn on its left side
     */
    DB::ResultId _leftDrop;

    /**
     * File which should have drop indication point drawn on its right side
     */
    DB::ResultId _rightDrop;

    QTimer* _repaintTimer;

    QMutex _pendingRepaintLock;
    IdSet _pendingRepaint;

    SortDirection _sortDirection;

    // For Shift + movement key selection handling
    Cell _cellOnFirstShiftMovementKey;
    IdSet _selectionOnFirstShiftMovementKey;

    bool _cursorWasAtStackIcon;

    ThumbnailCache _thumbnailCache;
};

}


#endif /* THUMBNAILVIEW_THUMBNAILWIDGET_H */

