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

#ifdef HASKIPI
#include "plugininterface.h"
#include <libkipi/imagecollection.h>
#include "myimagecollection.h"
#include "myimageinfo.h"
#include "imagedb.h"
#include "mainview.h"
#include "categoryimagecollection.h"
#include <klocale.h>
#include "imageinfo.h"

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
    if ( MainView::theMainView()->selected().count() != 0 )
        return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentSelection ) );
    else
        return KIPI::ImageCollection(0);
}

QValueList<KIPI::ImageCollection> PluginInterface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;
    ImageSearchInfo context = MainView::theMainView()->currentContext();
    QString category = MainView::theMainView()->currentBrowseCategory();
    if ( category.isNull() )
        category = Options::instance()->albumCategory();

    QMap<QString,int> categories = ImageDB::instance()->classify( context, category );

    for( QMapIterator<QString,int> it = categories.begin(); it != categories.end(); ++it ) {
        CategoryImageCollection* col = new CategoryImageCollection( context, category, it.key() );
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo PluginInterface::info( const KURL& url )
{
    return KIPI::ImageInfo( new MyImageInfo( this, url ) );
}

void PluginInterface::refreshImages( const KURL::List& urls )
{
    emit imagesChanged( urls );
}

int PluginInterface::features() const
{
    return
        KIPI::ImagesHasComments |
        KIPI::ImagesHasTime |
        KIPI::SupportsDateRanges |
        KIPI::AcceptNewImages |
        KIPI::ImageTitlesWritable;
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

void PluginInterface::delImage( const KURL& url )
{
    ImageInfo* info = ImageDB::instance()->find( url.path() );
    if ( info ) {
        ImageInfoList list;
        list.append( info );
        ImageDB::instance()->deleteList( list );
    }
}

void PluginInterface::slotSelectionChanged( bool b )
{
    emit selectionChanged( b );
}

#include "plugininterface.moc"
#endif // KIPI
