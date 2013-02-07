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
#ifndef THUMBNAILVIEW_THUMBNAILWIDGET_H
#define THUMBNAILVIEW_THUMBNAILWIDGET_H

#include <QListView>
#include "ThumbnailComponent.h"
#include "GridResizeInteraction.h"
#include "MouseTrackingInteraction.h"
#include "SelectionInteraction.h"
#include "ThumbnailView/enums.h"
#include <QScopedPointer>
#include "VideoThumbnailCycler.h"

class QTimer;
class QDateTime;

namespace DB { class ImageDate; class Id; class FileNameList;}


namespace ThumbnailView
{
class ThumbnailPainter;
class CellGeometry;
class ThumbnailModel;
class ThumbnailFactory;
class KeyboardEventHandler;
class ThumbnailDND;

class ThumbnailWidget : public QListView, private ThumbnailComponent {
    Q_OBJECT

public:
    ThumbnailWidget( ThumbnailFactory* factory );

    void reload( SelectionUpdateMethod method );
    DB::FileName mediaIdUnderCursor() const;
    QModelIndex indexUnderCursor() const;

    bool isMouseOverStackIndicator( const QPoint& point );
    bool isGridResizing() const;
    void setCurrentItem(  const DB::FileName& fileName );
    DB::FileName currentItem() const;
    void changeSingleSelection(const DB::FileName& fileName);

    // Misc
    int cellWidth() const;
    OVERRIDE void showEvent( QShowEvent* );
    DB::FileNameList selection( ThumbnailView::SelectionMode mode ) const;
    bool isSelected( const DB::FileName& id ) const;
    void select( const DB::FileNameList& );
    bool isItemUnderCursorSelected() const;

public slots:
    void gotoDate( const DB::ImageDate& date, bool includeRanges );

signals:
    void showImage( const DB::FileName& id );
    void showSelection();
    void fileIdUnderCursorChanged( const DB::FileName& id );
    void currentDateChanged( const QDateTime& );
    void selectionCountChanged(int numberOfItemsSelected );

protected:
    // event handlers
    OVERRIDE void keyPressEvent( QKeyEvent* );
    OVERRIDE void keyReleaseEvent( QKeyEvent* );
    OVERRIDE void mousePressEvent( QMouseEvent* );
    OVERRIDE void mouseMoveEvent( QMouseEvent* );
    OVERRIDE void mouseReleaseEvent( QMouseEvent* );
    OVERRIDE void mouseDoubleClickEvent ( QMouseEvent* );
    OVERRIDE void wheelEvent( QWheelEvent* );

    // Drag and drop
    OVERRIDE void dragEnterEvent ( QDragEnterEvent * event );
    OVERRIDE void dragMoveEvent ( QDragMoveEvent * );
    OVERRIDE void dragLeaveEvent ( QDragLeaveEvent * );
    OVERRIDE void dropEvent ( QDropEvent * );

private slots:
    void emitDateChange();
    void scheduleDateChangeSignal();
    void emitSelectionChangedSignal();

private:
    friend class GridResizeInteraction;
    inline ThumbnailModel* model() { return ThumbnailComponent::model(); }
    inline const ThumbnailModel* model() const { return ThumbnailComponent::model(); }
    void updatePalette();
    void setupDateChangeTimer();

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
    bool m_pressOnStackIndicator;

    QTimer* m_dateChangedTimer;

    friend class SelectionInteraction;
    friend class KeyboardEventHandler;
    friend class ThumbnailDND;
    friend class ThumbnailModel;
    KeyboardEventHandler* _keyboardHandler;
    QScopedPointer<VideoThumbnailCycler> m_videoThumbnailCycler;
};

}


#endif /* THUMBNAILVIEW_THUMBNAILWIDGET_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
