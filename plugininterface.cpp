#include "plugininterface.h"
#include <libkipi/imagecollection.h>
#include "myimagecollection.h"
#include "myimageinfo.h"

PluginInterface::PluginInterface( QObject *parent, const char *name )
    :KIPI::Interface( parent, name )
{
}

KIPI::ImageCollection PluginInterface::currentAlbum()
{
    return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentAlbum ) );
}

KIPI::ImageCollection PluginInterface::currentView()
{
    return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentView ) );
}

KIPI::ImageCollection PluginInterface::currentSelection()
{
    return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentSelection ) );
}

QValueList<KIPI::ImageCollection> PluginInterface::allAlbums()
{
    qDebug("QValueList<ImageCollection> PluginInterface::allAlbums() not implemented!");
    return QValueList<KIPI::ImageCollection>(); // PENDING(blackie) implement
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
    return KIPI::ImagesHasComments | KIPI::ImagesHasTime | KIPI::SupportsDateRanges;
}

#include "plugininterface.moc"
