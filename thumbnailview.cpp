#include "thumbnailview.h"
#include <qstringlist.h>
#include "thumbnail.h"
#include <qpixmap.h>
#include "viewer.h"
#include <qmessagebox.h>
#include "iconviewtooltip.h"
#include <klocale.h>

ThumbNailView::ThumbNailView( QWidget* parent, const char* name )
    :QIconView( parent,  name ), _currentHighlighted( 0 )
{
    _imageList=0L;
    setResizeMode( QIconView::Adjust );
    setAutoArrange( true );

    connect( this,  SIGNAL( returnPressed( QIconViewItem* ) ), this, SLOT( showImage( QIconViewItem* ) ) );
    connect( this,  SIGNAL( doubleClicked( QIconViewItem* ) ), this, SLOT( showImage( QIconViewItem* ) ) );

    setSelectionMode( Extended );
    new IconViewToolTip( this );
}


void ThumbNailView::showImage( QIconViewItem* item )
{
    if ( item ) {
        ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
        Q_ASSERT( tn );
        ImageInfoList list;
        list.append( tn->imageInfo() );
        Viewer* viewer = Viewer::instance();
        viewer->show();
        viewer->load( list );
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
            new ThumbNail( *it,  this,  "thumbnail" );
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
        _cutList.append( (*it)->imageInfo() );
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
    else if ( _cutList.count() == 0 ) {
        QMessageBox::information( this, i18n("Nothing on Clipboard"), i18n("<qt><p>No data on clipboard to paste.</p>"
                                  "<p>It really doesn't make any sence to the application to have an image represented twice, "
                                  "therefore you can only paste an image off the clipboard ones.</p>"),
                                  QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
    }
    else {
        ThumbNail* last = list.last();

        // Update the image list
        int index = _imageList->findRef( last->imageInfo() ) +1;
        for( ImageInfoListIterator it( _cutList ); *it; ++it ) {
            _imageList->insert( index, *it );
            ++index;
        }

        // updatet the thumbnail view
        for( ImageInfoListIterator it( _cutList ); *it; ++it ) {
            last = new ThumbNail( *it, last, this );
        }

        _cutList.clear();
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

bool ThumbNailView::isClipboardEmpty()
{
    return ( _cutList.count() == 0 );
}

ImageInfoList ThumbNailView::clipboard()
{
    return _cutList;
}

#include "thumbnailview.moc"
