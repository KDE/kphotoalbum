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
#ifndef THUMBNAILVIEW_THUMBNAILWIDGET_H
#define THUMBNAILVIEW_THUMBNAILWIDGET_H

#include "ThumbnailComponent.h"
#include <q3gridview.h>
#include "GridResizeInteraction.h"
#include "MouseTrackingInteraction.h"
#include "SelectionInteraction.h"
#include "ThumbnailView/enums.h"

class QTimer;
class QDateTime;

namespace DB { class ImageDate; class ResultId; }


namespace ThumbnailView
{
class ThumbnailPainter;
class CellGeometry;
class ThumbnailModel;
class ThumbnailFactory;
class KeyboardEventHandler;
class ThumbnailDND;
class Cell;

class ThumbnailWidget : public Q3GridView, private ThumbnailComponent {
    Q_OBJECT

public:
    ThumbnailWidget( ThumbnailFactory* factory );

    OVERRIDE void paintCell ( QPainter * p, int row, int col );

    void reload( bool flushCache, bool clearSelection=true );
    DB::ResultId mediaIdUnderCursor() const;

    // Painting
    void updateCell( const DB::ResultId& id );
    void updateCell( int row, int col );
    void updateCellSize();

    // Cell handling methods.
    Cell cellAtCoordinate( const QPoint& pos, CoordinateSystem ) const;

    void scrollToCell( const Cell& newPos );

    int firstVisibleRow( VisibleState ) const;
    int lastVisibleRow( VisibleState ) const;
    int numRowsPerPage() const;
    bool isFocusAtFirstCell() const;
    bool isFocusAtLastCell() const;
    Cell lastCell() const;
    bool isMouseOverStackIndicator( const QPoint& point );
    bool isGridResizing();

    // Misc
    void updateGridSize();
    QPoint viewportToContentsAdjusted( const QPoint& coordinate, CoordinateSystem system ) const;

public slots:
    void gotoDate( const DB::ImageDate& date, bool includeRanges );
    void repaintScreen();

signals:
    void showImage( const DB::ResultId& id );
    void showSelection();
    void fileIdUnderCursorChanged( const DB::ResultId& id );
    void currentDateChanged( const QDateTime& );
    void selectionChanged();

protected:
    OVERRIDE void viewportPaintEvent( QPaintEvent* );

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
    OVERRIDE void dimensionChange ( int oldNumRows, int oldNumCols );

    // Drag and drop
    OVERRIDE void contentsDragEnterEvent ( QDragEnterEvent * event );
    OVERRIDE void contentsDragMoveEvent ( QDragMoveEvent * );
    OVERRIDE void contentsDragLeaveEvent ( QDragLeaveEvent * );
    OVERRIDE void contentsDropEvent ( QDropEvent * );

    /**
     * For all filenames in the list, check if there are any missing
     * thumbnails and generate these in the background.
     */
    void generateMissingThumbnails(const DB::Result& items) const;

protected slots:
    void emitDateChange( int, int );
    void slotViewChanged( int, int );

private:
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

    GridResizeInteraction _gridResizeInteraction;
    bool _wheelResizing;
    SelectionInteraction _selectionInteraction;
    MouseTrackingInteraction _mouseTrackingHandler;
    MouseInteraction* _mouseHandler;
    ThumbnailDND* _dndHandler;

    friend class SelectionInteraction;
    friend class KeyboardEventHandler;
    friend class ThumbnailDND;
    KeyboardEventHandler* _keyboardHandler;
};

}


#endif /* THUMBNAILVIEW_THUMBNAILWIDGET_H */

