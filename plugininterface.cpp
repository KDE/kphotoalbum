#include "plugininterface.h"
#include <libkipi/imagecollection.h>
#include "myimagecollection.h"
#include "myimageinfo.h"
#include "imagedb.h"
#include "mainview.h"
#include "categoryimagecollection.h"
#include <klocale.h>

PluginInterface::PluginInterface( QObject *parent, const char *name )
    :KIPI::Interface( parent, name )
{
}

KIPI::ImageCollection PluginInterface::currentAlbum()
{
    return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentAlbum ) );
}

KIPI::ImageCollection PluginInterface::currentSelection()
{
    return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentSelection ) );
}

QValueList<KIPI::ImageCollection> PluginInterface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;
    ImageSearchInfo context = MainView::theMainView()->currentContext();
    QString optionGroup = MainView::theMainView()->currentBrowseCategory();
    if ( optionGroup.isNull() )
        optionGroup = Options::instance()->albumCategory();

    QMap<QString,int> categories = ImageDB::instance()->classify( context, optionGroup );

    for( QMapIterator<QString,int> it = categories.begin(); it != categories.end(); ++it ) {
        CategoryImageCollection* col = new CategoryImageCollection( context, optionGroup, it.key() );
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo PluginInterface::info( const KURL& url )
{
    return KIPI::ImageInfo( new MyImageInfo( url ) );
}

void PluginInterface::refreshImages( const KURL::List& urls )
{
    emit imagesChanged( urls );
}

int PluginInterface::features() const
{
    return KIPI::ImagesHasComments | KIPI::ImagesHasTime | KIPI::SupportsDateRanges |
        KIPI::AcceptNewImages;
}

bool PluginInterface::addImage( const KURL& url, QString& errmsg )
{
    QString dir = url.path();
    QString root = Options::instance()->imageDirectory();
    if ( !dir.startsWith( root ) ) {
        errmsg = i18n("<qt>Image needs to be placed in a sub directory of the KimDaBa image database, "
                      "which is rooted at %1. Image path was %2</qt>").arg( root ).arg( dir );
        return false;
    }

    dir = dir.mid( root.length() );
    ImageInfo* info = new ImageInfo( dir );
    ImageDB::instance()->addImage( info );
    return true;
}

#include "plugininterface.moc"
