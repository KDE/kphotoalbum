#include "thumbnailview.h"
#include <qstringlist.h>
#include "thumbnail.h"
#include <qpixmap.h>
#include "viewer.h"

ThumbNailView::ThumbNailView( QWidget* parent, const char* name )
    :QIconView( parent,  name )
{
    setResizeMode( QIconView::Adjust );

    connect( this,  SIGNAL( returnPressed( QIconViewItem* ) ),  this,  SLOT( showImage( QIconViewItem* ) ) );
    setSelectionMode( Extended );
    setItemsMovable( false );
}


void ThumbNailView::showImage( QIconViewItem* item )
{
    if ( item ) {
        ThumbNail* tn = dynamic_cast<ThumbNail*>( item );
        Q_ASSERT( tn );
        Viewer* viewer = new Viewer( this );
        viewer->show();
        viewer->load(  tn->imageInfo() );
    }
}

void ThumbNailView::startDrag()
{
    // No dnd please.
    return;
}

void ThumbNailView::load( ImageInfoList* list )
{
    for( QPtrListIterator<ImageInfo> it( *list ); *it; ++it ) {
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
