/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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

ThumbNailView::ThumbNailView( QWidget* parent, const char* name )
    :KIconView( parent,  name ), _currentHighlighted( 0 )
{
    setSpacing(2);

    setResizeMode( QIconView::Adjust );
    setAutoArrange( true );

    connect( this, SIGNAL( returnPressed( QIconViewItem* ) ), this, SLOT( showImage( QIconViewItem* ) ) );
    connect( this, SIGNAL( executed( QIconViewItem* ) ), this, SLOT( showImage( QIconViewItem* ) ) );
    setSelectionMode( Extended );
    _iconViewToolTip = new IconViewToolTip( this );
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
                // We don't want this to be child of anything. Originally it was child of the mainwindow
                // but that had the effect that it would always be on top of it.
                viewer = new Viewer( 0 );
                viewer->show();
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
    clear();
    ImageInfoList& list = ImageDB::instance()->images();
    if ( list.isEmpty() )
        return;

    clear();
    for( QPtrListIterator<ImageInfo> it( list ); *it; ++it ) {
        if ( (*it)->visible() )
            new ThumbNail( *it,  this );
    }
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
    else
        repaintContents( 0,0, width(), height(), true );
}

void ThumbNailView::setHighlighted( ThumbNail* item )
{
    if ( _currentHighlighted == item )
        return;

    if ( _currentHighlighted )
        _currentHighlighted->dragLeft();
    _currentHighlighted = item;
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
        QMessageBox::information( this, i18n("Nothing Selected"), i18n("To paste you have to select an image that the past should go after."), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
    }
    else if ( ImageDB::instance()->clipboard().count() == 0 ) {
        QMessageBox::information( this, i18n("Nothing on Clipboard"), i18n("<qt><p>No data on clipboard to paste.</p>"
                                  "<p>It really doesn't make any sense to the application to have an image represented twice, "
                                  "therefore you can only paste an image off the clipboard once.</p>"),
                                  QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
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

void ThumbNailView::showToolTipsOnImages()
{
    _iconViewToolTip->showToolTips();
}


QDragObject* ThumbNailView::dragObject()
{
    QPtrList<ThumbNail> list= selected();
    if ( !list.isEmpty() ) {
        KURL::List l;
        for( QPtrListIterator<ThumbNail> it( list ); *it; ++it ) {
            l.append( (*it)->fileName() );
        }
        return new KURLDrag( l, this );
    }

    else
        return 0;
}

#include "thumbnailview.moc"
