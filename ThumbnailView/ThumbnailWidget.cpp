/* Copyright (C) 2003-2011 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailWidget.h"
#include <QScrollBar>
#include <QTimer>
#include "Delegate.h"
#include "ImageManager/ThumbnailCache.h"
#include "ThumbnailDND.h"
#include "KeyboardEventHandler.h"
#include "ThumbnailFactory.h"
#include "ThumbnailModel.h"
#include "CellGeometry.h"
#include "ThumbnailWidget.moc"
#include <math.h>

#include <klocale.h>
#include <qcursor.h>
#include <qfontmetrics.h>
#include <qpainter.h>

#include "Browser/BrowserWidget.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfoPtr.h"
#include "Settings/SettingsData.h"
#include "Utilities/Set.h"
#include "Utilities/Util.h"
#include "SelectionMaintainer.h"

/**
 * \class ThumbnailView::ThumbnailWidget
 * This is the widget which shows the thumbnails.
 *
 * In previous versions this was implemented using a QIconView, but there
 * simply was too many problems, so after years of tears and pains I
 * rewrote it.
 */
using Utilities::StringSet;

ThumbnailView::ThumbnailWidget::ThumbnailWidget( ThumbnailFactory* factory)
    :QListView(),
     ThumbnailComponent( factory ),
     _isSettingDate(false),
     _gridResizeInteraction( factory ),
     _wheelResizing( false ),
     _selectionInteraction( factory ),
     _mouseTrackingHandler( factory ),
     _mouseHandler( &_mouseTrackingHandler ),
     _dndHandler( new ThumbnailDND( factory ) ),
     m_pressOnStackIndicator( false ),
     _keyboardHandler( new KeyboardEventHandler( factory ) ),
      m_videoThumbnailCycler( new VideoThumbnailCycler(model()) )
{
    setModel( ThumbnailComponent::model() );
    setResizeMode( QListView::Adjust );
    setViewMode( QListView::IconMode );
    setUniformItemSizes(true);
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    // It beats me why I need to set mouse tracking on both, but without it doesn't work.
    viewport()->setMouseTracking( true );
    setMouseTracking( true );

    connect( selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex) ), this, SLOT( scheduleDateChangeSignal() ) );
    viewport()->setAcceptDrops( true );

    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    connect( &_mouseTrackingHandler, SIGNAL( fileIdUnderCursorChanged( DB::FileName ) ), this, SIGNAL( fileIdUnderCursorChanged( DB::FileName ) ) );
    connect( _keyboardHandler, SIGNAL( showSelection() ), this, SIGNAL( showSelection() ) );

    updatePalette();
    setItemDelegate( new Delegate(factory) );

    connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( emitSelectionChangedSignal() ) );

    setDragEnabled(false); // We run our own dragging, so disable QListView's version.

    connect( verticalScrollBar(), SIGNAL( valueChanged(int) ), model(), SLOT( updateVisibleRowInfo() ) );
    setupDateChangeTimer();
}

bool ThumbnailView::ThumbnailWidget::isGridResizing() const
{
    return _mouseHandler->isResizingGrid() || _wheelResizing;
}

void ThumbnailView::ThumbnailWidget::keyPressEvent( QKeyEvent* event )
{
    if ( !_keyboardHandler->keyPressEvent( event ) )
        QListView::keyPressEvent( event );
}

void ThumbnailView::ThumbnailWidget::keyReleaseEvent( QKeyEvent* event )
{
    const bool propagate = _keyboardHandler->keyReleaseEvent( event );
    if ( propagate )
        QListView::keyReleaseEvent(event);
}



bool ThumbnailView::ThumbnailWidget::isMouseOverStackIndicator( const QPoint& point )
{
    // first check if image is stack, if not return.
    DB::ImageInfoPtr imageInfo = mediaIdUnderCursor().info();
    if (!imageInfo) return false;
    if (!imageInfo->isStacked()) return false;

    const QModelIndex index = indexUnderCursor();
    const QRect itemRect = visualRect( index );
    const QPixmap pixmap = index.data( Qt::DecorationRole ).value<QPixmap>();
    if ( pixmap.isNull() )
        return false;

    const QRect pixmapRect = cellGeometryInfo()->iconGeometry( pixmap ).translated( itemRect.topLeft() );
    const QRect blackOutRect = pixmapRect.adjusted( 0,0, -10, -10 );
    return pixmapRect.contains(point) && !blackOutRect.contains( point );
}

static bool isMouseResizeGesture( QMouseEvent* event )
{
    return
        (event->button() & Qt::MidButton) ||
        ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::AltModifier));
}

void ThumbnailView::ThumbnailWidget::mousePressEvent( QMouseEvent* event )
{
    if ( (!(event->modifiers() & ( Qt::ControlModifier | Qt::ShiftModifier ) )) && isMouseOverStackIndicator( event->pos() ) ) {
        model()->toggleStackExpansion(mediaIdUnderCursor());
        m_pressOnStackIndicator = true;
        return;
    }

    if ( isMouseResizeGesture( event ) )
        _mouseHandler = &_gridResizeInteraction;
    else
        _mouseHandler = &_selectionInteraction;

    if ( !_mouseHandler->mousePressEvent( event ) )
        QListView::mousePressEvent( event );

    if (event->button() & Qt::RightButton) //get out of selection mode if this is a right click
      _mouseHandler = &_mouseTrackingHandler;

}

void ThumbnailView::ThumbnailWidget::mouseMoveEvent( QMouseEvent* event )
{
    if ( m_pressOnStackIndicator )
        return;

    if ( !_mouseHandler->mouseMoveEvent( event ) )
        QListView::mouseMoveEvent( event );
}

void ThumbnailView::ThumbnailWidget::mouseReleaseEvent( QMouseEvent* event )
{
    if ( m_pressOnStackIndicator ) {
        m_pressOnStackIndicator = false;
        return;
    }

    if ( !_mouseHandler->mouseReleaseEvent( event ) )
        QListView::mouseReleaseEvent( event );

    _mouseHandler = &_mouseTrackingHandler;
}

void ThumbnailView::ThumbnailWidget::mouseDoubleClickEvent( QMouseEvent * event )
{
    if ( isMouseOverStackIndicator( event->pos() ) ) {
        model()->toggleStackExpansion(mediaIdUnderCursor());
        m_pressOnStackIndicator = true;
    } else if ( !( event->modifiers() & Qt::ControlModifier ) ) {
        DB::FileName id = mediaIdUnderCursor();
        if ( !id.isNull() )
            emit showImage( id );
    }
}

void ThumbnailView::ThumbnailWidget::wheelEvent( QWheelEvent* event )
{
    if ( event->modifiers() & Qt::ControlModifier ) {
        event->setAccepted(true);
        if ( !_wheelResizing)
            _gridResizeInteraction.enterGridResizingMode();

        _wheelResizing = true;

        const int delta = -event->delta() / 20;
        Settings::SettingsData::instance()->setThumbSize( qMax( 32, Settings::SettingsData::instance()->thumbSize() + delta ) );
        cellGeometryInfo()->calculateCellSize();
        model()->reset();
    }
    else
    {
        int delta = event->delta() / 5;
        QWheelEvent newevent = QWheelEvent(event->pos(), delta, event->buttons(), NULL);

        QListView::wheelEvent(&newevent);
    }
}


void ThumbnailView::ThumbnailWidget::emitDateChange()
{
    if ( _isSettingDate )
        return;

    int row = currentIndex().row();
    if (row == -1)
    return;

    DB::FileName fileName = model()->imageAt( row );
    if ( fileName.isNull() )
        return;

    static QDateTime lastDate;
    QDateTime date = fileName.info()->date().start();
    if ( date != lastDate ) {
        lastDate = date;
        if ( date.date().year() != 1900 )
            emit currentDateChanged( date );
    }
}

/**
 * scroll to the date specified with the parameter date.
 * The boolean includeRanges tells whether we accept range matches or not.
 */
void ThumbnailView::ThumbnailWidget::gotoDate( const DB::ImageDate& date, bool includeRanges )
{
    _isSettingDate = true;
    DB::FileName candidate = DB::ImageDB::instance()
                             ->findFirstItemInRange(model()->imageList(ViewOrder), date, includeRanges);
    if ( !candidate.isNull() )
        setCurrentItem( candidate );

    _isSettingDate = false;
}


void ThumbnailView::ThumbnailWidget::reload(SelectionUpdateMethod method )
{
    SelectionMaintainer maintainer( this, model());
    cellGeometryInfo()->flushCache();
    updatePalette();

    // const DB::IdList selectedItems = selection( NoExpandCollapsedStacks );
    // PENDING(blackie) the selection wasn't used
    ThumbnailComponent::model()->reset();

    if ( method == ClearSelection )
        maintainer.disable();
}

DB::FileName ThumbnailView::ThumbnailWidget::mediaIdUnderCursor() const
{
    const QModelIndex index = indexUnderCursor();
    if ( index.isValid() )
        return model()->imageAt(index.row());
    else
        return DB::FileName();
}

QModelIndex ThumbnailView::ThumbnailWidget::indexUnderCursor() const
{
    return indexAt( mapFromGlobal( QCursor::pos() ) );
}


void ThumbnailView::ThumbnailWidget::dragMoveEvent( QDragMoveEvent* event )
{
    _dndHandler->contentsDragMoveEvent(event);
}

void ThumbnailView::ThumbnailWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
    _dndHandler->contentsDragLeaveEvent( event );
}

void ThumbnailView::ThumbnailWidget::dropEvent( QDropEvent* event )
{
    _dndHandler->contentsDropEvent( event );
}


void ThumbnailView::ThumbnailWidget::dragEnterEvent( QDragEnterEvent * event )
{
    _dndHandler->contentsDragEnterEvent( event );
}

void ThumbnailView::ThumbnailWidget::setCurrentItem( const DB::FileName& fileName )
{
    if ( fileName.isNull() )
        return;

    const int row = model()->indexOf(fileName);
    setCurrentIndex( QListView::model()->index( row, 0 ) );
}

DB::FileName ThumbnailView::ThumbnailWidget::currentItem() const
{
    if ( !currentIndex().isValid() )
        return DB::FileName();

    return model()->imageAt( currentIndex().row());
}

void ThumbnailView::ThumbnailWidget::updatePalette()
{
    QPalette pal = palette();
    pal.setBrush( QPalette::Base, QColor(Settings::SettingsData::instance()->backgroundColor()) );
    pal.setBrush( QPalette::Text, Utilities::contrastColor( QColor(Settings::SettingsData::instance()->backgroundColor() ) ) );
    setPalette( pal );
}

int ThumbnailView::ThumbnailWidget::cellWidth() const
{
    return visualRect( QListView::model()->index(0,0) ).size().width();
}

void ThumbnailView::ThumbnailWidget::emitSelectionChangedSignal()
{
    emit selectionCountChanged( selection( ExpandCollapsedStacks ).size() );
}

void ThumbnailView::ThumbnailWidget::scheduleDateChangeSignal()
{
    m_dateChangedTimer->start(200);
}

/**
 * During profiling, I found that emitting the dateChanged signal was
 * rather expensive, so now I delay that signal, so it is only emitted 200
 * msec after the scroll, which means it will not be emitted when the user
 * holds down, say the page down key for scrolling.
 */
void ThumbnailView::ThumbnailWidget::setupDateChangeTimer()
{
    m_dateChangedTimer = new QTimer;
    m_dateChangedTimer->setSingleShot(true);
    connect( m_dateChangedTimer, SIGNAL(timeout()), this, SLOT( emitDateChange() ) );
}

void ThumbnailView::ThumbnailWidget::showEvent( QShowEvent* event )
{
    model()->updateVisibleRowInfo();
    QListView::showEvent( event );
}

DB::FileNameList ThumbnailView::ThumbnailWidget::selection( ThumbnailView::SelectionMode mode ) const
{
    DB::FileNameList res;
    Q_FOREACH(const QModelIndex& index, selectedIndexes()) {
        DB::FileName currFileName = model()->imageAt(index.row());
        bool includeAllStacks = false;
        switch ( mode )
        {
            case IncludeAllStacks:
                includeAllStacks = true;
                // no break!
            case ExpandCollapsedStacks:
                {
                    // if the selected image belongs to a collapsed thread,
                    // imply that all images in the stack are selected:
                    DB::ImageInfoPtr imageInfo = currFileName.info();
                    if ( imageInfo && imageInfo->isStacked()
                            && ( includeAllStacks || ! model()->isItemInExpandedStack( imageInfo->stackId() ) ) 
                            )
                    {
                        // add all images in the same stack
                        res.append(DB::ImageDB::instance()->getStackFor(currFileName));
                    } else
                        res.append(currFileName);
                } 
                break;
            case NoExpandCollapsedStacks:
                res.append(currFileName);
                break;
        }
    }
    return res;
}

bool ThumbnailView::ThumbnailWidget::isSelected( const DB::FileName& fileName ) const
{
    return selection( NoExpandCollapsedStacks ).indexOf(fileName) != -1;
}

/**
   This very specific method will make the item specified by id selected,
   if there only are one item selected. This is used from the Viewer when
   you start it without a selection, and are going forward or backward.
*/
void ThumbnailView::ThumbnailWidget::changeSingleSelection(const DB::FileName& fileName)
{
    if ( selection( NoExpandCollapsedStacks ).size() == 1 ) {
        QItemSelectionModel* selection = selectionModel();
        selection->select( model()->fileNameToIndex(fileName), QItemSelectionModel::ClearAndSelect );
        setCurrentItem(fileName);
    }
}

void ThumbnailView::ThumbnailWidget::select(const DB::FileNameList& items )
{
    Q_FOREACH( const DB::FileName& fileName, items )
        selectionModel()->select(model()->fileNameToIndex(fileName), QItemSelectionModel::Select);
}

bool ThumbnailView::ThumbnailWidget::isItemUnderCursorSelected() const
{
    return widget()->selection(ExpandCollapsedStacks).contains(mediaIdUnderCursor());
}

// vi:expandtab:tabstop=4 shiftwidth=4:
