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

ThumbNailView::ThumbNailView( QWidget* parent, const char* name )
    :KIconView( parent,  name ), _currentHighlighted( 0 )
{
    setSpacing(2);


    _imageList=0L;
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

void ThumbNailView::load( ImageInfoList* list )
{
    if ( !list )
        return;
    clear();
    for( QPtrListIterator<ImageInfo> it( *list ); *it; ++it ) {
        if ( (*it)->visible() )
            new ThumbNail( *it,  this );
    }
    _imageList = list;
}

void ThumbNailView::reload()
{
    clear();
    load( _imageList );
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

void ThumbNailView::reorder( ImageInfo* item, const ImageInfoList& list, bool after )
{
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        _imageList->removeRef( *it );
    }

    int index =  _imageList->find( item );
    if ( after )
        ++index;

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        _imageList->insert( index, *it );
        ++index;
    }
    emit changed();
}

void ThumbNailView::contentsDropEvent( QDropEvent* e )
{
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
    QPtrList<ThumbNail> list = selected();
    for( QPtrListIterator<ThumbNail> it( list ); *it; ++it ) {
        ImageDB::instance()->clipboard().append( (*it)->imageInfo() );
        _imageList->removeRef( (*it)->imageInfo() );
        delete *it;
    }
}

void ThumbNailView::slotPaste()
{
    QPtrList<ThumbNail> list = selected();
    if ( list.count() == 0 ) {
        QMessageBox::information( this, i18n("Nothing Selected"), i18n("To paste you have to select an image that the past should go after."), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
    }
    else if ( ImageDB::instance()->clipboard().count() == 0 ) {
        QMessageBox::information( this, i18n("Nothing on Clipboard"), i18n("<qt><p>No data on clipboard to paste.</p>"
                                  "<p>It really doesn't make any sense to the application to have an image represented twice, "
                                  "therefore you can only paste an image off the clipboard ones.</p>"),
                                  QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
    }
    else {
        ThumbNail* last = list.last();

        // Update the image list
        int index = _imageList->findRef( last->imageInfo() ) +1;
        ImageInfoList& list = ImageDB::instance()->clipboard();
        for( ImageInfoListIterator it( list ); *it; ++it ) {
            _imageList->insert( index, *it );
            ++index;
        }

        // updatet the thumbnail view
        for( ImageInfoListIterator it( list ); *it; ++it ) {
            last = new ThumbNail( *it, last, this );
        }

        list.clear();
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


#include "thumbnailview.moc"
