#include "thumbnailview.h"
#include <qstringlist.h>
#include "thumbnail.h"
#include <qpixmap.h>
#include "viewer.h"

ThumbNailView::ThumbNailView( QWidget* parent, const char* name )
    :QIconView( parent,  name ), _currentHighlighted( 0 )
{
    setResizeMode( QIconView::Adjust );
    setAutoArrange( true );

    connect( this,  SIGNAL( returnPressed( QIconViewItem* ) ),  this,  SLOT( showImage( QIconViewItem* ) ) );
    connect( this,  SIGNAL( doubleClicked( QIconViewItem* ) ),  this,  SLOT( showImage( QIconViewItem* ) ) );

    setSelectionMode( Extended );
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

//void ThumbNailView::contentsDragEnterEvent( QDragEnterEvent *e )
//{
//    e->ignore();
//}
