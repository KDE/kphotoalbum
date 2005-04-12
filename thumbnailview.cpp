/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "thumbnailview.h"
#include <qstringlist.h>
#include "thumbnail.h"
#include <qpixmap.h>
#include "viewer.h"
#include <qmessagebox.h>
#include "iconviewtooltip.h"
#include <klocale.h>
#include "imagedb.h"
#include <qapplication.h>
#include "util.h"
#include <qpopupmenu.h>
#include <kurldrag.h>
#include <kmessagebox.h>
#include <qpainter.h>
#include "imagedaterange.h"
#include "imageinfo.h"
#include <qpixmapcache.h>

ThumbNailView* ThumbNailView::_instance = 0;

ThumbNailView::ThumbNailView( QWidget* parent, const char* name )
    :KIconView( parent,  name ), _currentHighlighted( 0 ), _blockMoveSignals( false )
{
    Q_ASSERT( !_instance );
    _instance = this;

    setResizeMode( QIconView::Adjust );
    setAutoArrange( true );

    connect( this, SIGNAL( returnPressed( QIconViewItem* ) ), this, SLOT( showImage( QIconViewItem* ) ) );
    connect( this, SIGNAL( executed( QIconViewItem* ) ), this, SLOT( showImage( QIconViewItem* ) ) );
    setSelectionMode( Extended );
    _iconViewToolTip = new IconViewToolTip( this );
    connect( this, SIGNAL( onItem( QIconViewItem* ) ), this, SLOT( slotOnItem( QIconViewItem* ) ) );
    connect( this, SIGNAL( onViewport() ), this, SLOT( slotOnViewPort() ) );
    setupGrid();
    connect( Options::instance(), SIGNAL( changed() ), this, SLOT( setupGrid() ) );
    connect( this, SIGNAL( contentsMoving(int, int) ), this, SLOT( emitDateChange() ) );
}


void ThumbNailView::showImage( QIconViewItem* item )
{
    if ( item ) {
        ThumbNail* tn = static_cast<ThumbNail*>( item );
        ImageInfo* info = tn->imageInfo();
        if ( !info->imageOnDisk() ) {
            QMessageBox::warning( this, i18n("No Images to Display"),
                                  i18n("The selected image was not available on disk.") );
        }
        else {
            ImageInfoList list;
            list.append( info );
            Viewer* viewer;
            if ( !Util::ctrlKeyDown() && Viewer::latest() ) {
                viewer = Viewer::latest();
                viewer->setActiveWindow();
                viewer->raise();
            }
            else {
                viewer = new Viewer( "viewer" );
                viewer->show( false );
            }
            viewer->load( list );
        }
    }
}

void ThumbNailView::startDrag()
{
    // No dnd please.
    QIconView::startDrag();
    return;
}

void ThumbNailView::reload()
{
    ThumbNail::pixmapCache().clear();
    // I'm not sure if this is needed, it would require that we were in a
    // drag'n'drop action, and the sudantly got the reload before we got
    // the drop event or leave event.
    _currentHighlighted = 0;

    clear();
    _iconViewToolTip->clear();
    ImageInfoList& list = ImageDB::instance()->images();
    if ( list.isEmpty() )
        return;

    ThumbNail* first = 0;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        if ( (*it)->visible() ) {
            ThumbNail* tn = new ThumbNail( *it,  this );
            if ( !first )
                first = tn;
        }
    }
    if ( first ) {
        first->setSelected( true );
        setCurrentItem( first );
    }

    emitDateChange();
}

void ThumbNailView::slotSelectAll()
{
    selectAll( true );
}

void ThumbNailView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    QIconView::contentsDragMoveEvent( e );
    QIconViewItem* item = findItem( e->pos() );
    if ( item ) {
        ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
        Q_ASSERT( tn );
        tn->dragMove();
    }
}

void ThumbNailView::reorder( ImageInfo* item, const ImageInfoList& cutList, bool after )
{
    ImageInfoList& images = ImageDB::instance()->images();

    for( ImageInfoListIterator it( cutList ); *it; ++it ) {
        images.removeRef( *it );
    }

    int index =  images.find( item );
    if ( after )
        ++index;

    for( ImageInfoListIterator it( cutList ); *it; ++it ) {
        images.insert( index, *it );
        ++index;
    }
    emit changed();
}

void ThumbNailView::contentsDropEvent( QDropEvent* e )
{
    _currentHighlighted = 0;

    QIconViewItem* item = findItem( e->pos() );
    if ( item )
        QIconView::contentsDropEvent( e );
    else {
        repaintContents( 0,0, width(), height(), true );
        KMessageBox::information( this, i18n("Please drop onto an image.") );
    }
}

void ThumbNailView::setHighlighted( ThumbNail* item )
{
    if ( _currentHighlighted == item )
        return;

    if ( _currentHighlighted )
        _currentHighlighted->dragLeft();
    _currentHighlighted = item;
}

void ThumbNailView::setDragLeft( ThumbNail* item )
{
    if ( _currentHighlighted == item )
        _currentHighlighted = 0;
}


void ThumbNailView::slotCut()
{
    ImageInfoList& images = ImageDB::instance()->images();
    QPtrList<ThumbNail> thumbNails = selected();
    for( QPtrListIterator<ThumbNail> it( thumbNails ); *it; ++it ) {
        ImageDB::instance()->clipboard().append( (*it)->imageInfo() );
        images.removeRef( (*it)->imageInfo() );
        delete *it;
    }
}

void ThumbNailView::slotPaste()
{
    QPtrList<ThumbNail> selectedList = selected();
    if ( selectedList.count() == 0 ) {
        KMessageBox::information( this, i18n("To paste you have to select an image that the past should go after."), i18n("Nothing Selected") );
    }
    else if ( ImageDB::instance()->clipboard().count() == 0 ) {
        KMessageBox::information( this, i18n("<qt><p>No data on clipboard to paste.</p>"
                                  "<p>It really does not make any sense to the application to have an image represented twice, "
                                  "therefore you can only paste an image off the clipboard once.</p>"),
                                  i18n("Nothing on Clipboard") );
    }
    else {
        ThumbNail* last = selectedList.last();

        // Update the image list
        ImageInfoList& images = ImageDB::instance()->images();
        int index = images.findRef( last->imageInfo() ) +1;
        ImageInfoList& clipboard = ImageDB::instance()->clipboard();
        for( ImageInfoListIterator it( clipboard ); *it; ++it ) {
            images.insert( index, *it );
            ++index;
        }

        // updatet the thumbnail view
        for( ImageInfoListIterator it( clipboard ); *it; ++it ) {
            last = new ThumbNail( *it, last, this );
        }

        clipboard.clear();
        emit changed();
    }
}

QPtrList<ThumbNail> ThumbNailView::selected() const
{
    QPtrList<ThumbNail> list;
    for ( QIconViewItem* item = firstItem(); item; item = item->nextItem() ) {
        if ( item->isSelected() ) {
            ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
            Q_ASSERT( tn );
            list.append( tn );
        }
    }
    return list;
}

void ThumbNailView::showToolTipsOnImages( bool on )
{
    _iconViewToolTip->setActive( on );
}


QDragObject* ThumbNailView::dragObject()
{
    QPtrList<ThumbNail> list= selected();
    if ( !list.isEmpty() ) {
        KURL::List l;
        for( QPtrListIterator<ThumbNail> it( list ); *it; ++it ) {
            l.append( KURL((*it)->fileName()) );
        }
        return new KURLDrag( l, this, "drag" );
    }

    else
        return 0;
}

void ThumbNailView::slotOnItem( QIconViewItem* item )
{
    if ( item ) {
        ThumbNail* tn = static_cast<ThumbNail*>( item );
        emit fileNameChanged( tn->fileName() );
    }
}

void ThumbNailView::slotOnViewPort()
{
    emit fileNameChanged( QString::fromLatin1("") );
}

void ThumbNailView::setupGrid()
{
    int size = Options::instance()->thumbSize();
    if ( Options::instance()->alignColumns() )
        setGridX( size + 2 );
    else
        setGridX( -1 );

    setGridY(-1);
    setSpacing( Options::instance()->rowSpacing() );

}

void ThumbNailView::drawBackground( QPainter * p, const QRect & r )
{
    p->fillRect( r, Options::instance()->thumbNailBackgroundColor() );
}

void ThumbNailView::gotoDate( const ImageDateRange& date, bool includeRanges )
{
    // When the user clicks outside the range in the datebar, then first a
    // reload() is executed, and then this function, but when this
    // function is called, the thumbnails have not yet been placed, and
    // thus candiate->x() and candidate->y() below will not return valid values.
    qApp->processEvents();
    bool block = _blockMoveSignals;
    _blockMoveSignals = true;
    ThumbNail* candidate = 0;
    for ( QIconViewItem* item = firstItem(); item; item = item->nextItem() ) {
        ThumbNail* tn = static_cast<ThumbNail*>( item );
        ImageDateRange::MatchType match = tn->imageInfo()->dateRange().isIncludedIn( date );
        if ( match == ImageDateRange::ExactMatch || ( match == ImageDateRange::RangeMatch && includeRanges ) ) {
            if ( candidate ) {
                if ( tn->imageInfo()->startDate().min() < candidate->imageInfo()->startDate().min() )
                    candidate = tn;
            }
            else
                candidate = tn;
        }
    }
    if ( candidate ) {
        setContentsPos( candidate->x()+4, candidate->y()+4 );
        setCurrentItem( candidate );
        candidate->setSelected( true );
    }

    _blockMoveSignals = block;
}

ThumbNailView* ThumbNailView::theThumbnailView()
{
    return _instance;
}

void ThumbNailView::makeCurrent( ImageInfo* info )
{
    for ( QIconViewItem* item = firstItem(); item; item = item->nextItem() ) {
        ThumbNail* tn = static_cast<ThumbNail*>( item );
        if ( tn->imageInfo() == info ) {
            setCurrentItem( tn );
            tn->setSelected( true );
            ensureItemVisible( tn );
        }
    }
}

void ThumbNailView::emitDateChange()
{
    if ( _blockMoveSignals )
        return;

    QIconViewItem* item = findFirstVisibleItem( QRect( contentsX(), contentsY(), width(), height() ) );
    if ( item ) {
        ThumbNail* tn = static_cast<ThumbNail*>( item );
        QDateTime date = tn->imageInfo()->startDate().min();
        if ( date.date().year() != 1900 )
            emit currentDateChanged( date );
    }
}

void ThumbNailView::showEvent( QShowEvent* event )
{
    QIconView::showEvent( event );
    emitDateChange();
}

#include "thumbnailview.moc"
