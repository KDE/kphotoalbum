#include "browseritemfactory.h"
#include "folder.h"
#include <klocale.h>
BrowserIconViewItemFactory::BrowserIconViewItemFactory( QIconView* view )
    :BrowserItemFactory(), _view( view )
{
}

void BrowserIconViewItemFactory::createItem( Folder* folder )
{
    new BrowserIconItem( _view, folder );
}

BrowserListViewItemFactory::BrowserListViewItemFactory( QListView* view )
    :BrowserItemFactory(), _view( view )
{
}

void BrowserListViewItemFactory::createItem( Folder* folder )
{
    new BrowserListItem( _view, folder );
}

BrowserIconItem::BrowserIconItem( QIconView* view, Folder* folder )
    :QIconViewItem( view ), _folder(folder)
{
    setPixmap( folder->pixmap() );
    int count = folder->count();
    if ( count == -1 )
        setText( folder->text() );
    else
        setText( QString::fromLatin1( "%1 (%2)" ).arg( folder->text() ).arg( count ) );
}

BrowserListItem::BrowserListItem( QListView* view, Folder* folder )
     : QListViewItem( view ), _folder(folder)
{
    setPixmap( 0, folder->pixmap() );
    setText( 0, folder->text() );
    int count = folder->count();
    if ( count != -1 )
        setText( 1, i18n( "1 image", "%n images", count) );
}

int BrowserListItem::compare( QListViewItem* other, int col, bool asc ) const
{
    return _folder->compare( static_cast<BrowserListItem*>(other)->_folder, col, asc );
}

BrowserIconItem::~BrowserIconItem()
{
    delete _folder;
}

BrowserListItem::~BrowserListItem()
{
    delete _folder;
}

