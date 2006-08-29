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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#ifdef HASKIPI
#include "Plugins/Interface.h"
#include <libkipi/imagecollection.h>
#include "Plugins/ImageCollection.h"
#include "Plugins/ImageInfo.h"
#include "DB/ImageDB.h"
#include "MainWindow/Window.h"
#include "Plugins/CategoryImageCollection.h"
#include <klocale.h>
#include "DB/ImageInfo.h"
#include "Browser/BrowserWidget.h"

Plugins::Interface::Interface( QObject *parent, const char *name )
    :KIPI::Interface( parent, name )
{
    connect( Browser::BrowserWidget::instance(), SIGNAL( pathChanged( const QString& ) ), this, SLOT( pathChanged( const QString& ) ) );
}

KIPI::ImageCollection Plugins::Interface::currentAlbum()
{
    return KIPI::ImageCollection( new Plugins::ImageCollection( Plugins::ImageCollection::CurrentAlbum ) );
}

KIPI::ImageCollection Plugins::Interface::currentSelection()
{
    if ( MainWindow::Window::theMainWindow()->selected().count() != 0 )
        return KIPI::ImageCollection( new Plugins::ImageCollection( Plugins::ImageCollection::CurrentSelection ) );
    else
        return KIPI::ImageCollection(0);
}

QValueList<KIPI::ImageCollection> Plugins::Interface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;
    DB::ImageSearchInfo context = MainWindow::Window::theMainWindow()->currentContext();
    QString category = MainWindow::Window::theMainWindow()->currentBrowseCategory();
    if ( category.isNull() )
        category = Settings::SettingsData::instance()->albumCategory();

    QMap<QString,uint> categories = DB::ImageDB::instance()->classify( context, category, DB::Image );

    for( QMapIterator<QString,uint> it = categories.begin(); it != categories.end(); ++it ) {
        CategoryImageCollection* col = new CategoryImageCollection( context, category, it.key() );
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo Plugins::Interface::info( const KURL& url )
{
    return KIPI::ImageInfo( new Plugins::ImageInfo( this, url ) );
}

void Plugins::Interface::refreshImages( const KURL::List& urls )
{
    emit imagesChanged( urls );
}

int Plugins::Interface::features() const
{
    return
        KIPI::ImagesHasComments |
        KIPI::ImagesHasTime |
        KIPI::SupportsDateRanges |
        KIPI::AcceptNewImages |
        KIPI::ImageTitlesWritable;
}

bool Plugins::Interface::addImage( const KURL& url, QString& errmsg )
{
    QString dir = url.path();
    QString root = Settings::SettingsData::instance()->imageDirectory();
    if ( !dir.startsWith( root ) ) {
        errmsg = i18n("<qt>Image needs to be placed in a sub directory of PhotoAlbum, "
                      "which is rooted at %1. Image path was %2</qt>").arg( root ).arg( dir );
        return false;
    }

    dir = dir.mid( root.length() );
    DB::ImageInfoPtr info = new DB::ImageInfo( dir );
    DB::ImageInfoList list;
    list.append( info );
    DB::ImageDB::instance()->addImages( list );
    return true;
}

void Plugins::Interface::delImage( const KURL& url )
{
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( url.path() );
    if ( info ) {
        QStringList list;
        list.append( info->fileName() );
        DB::ImageDB::instance()->deleteList( list );
    }
}

void Plugins::Interface::slotSelectionChanged( bool b )
{
    emit selectionChanged( b );
}

void Plugins::Interface::pathChanged( const QString& path )
{
    static QString _path;
    if ( _path != path ) {
        emit currentAlbumChanged( true );
        _path = path;
    }
}

#include "Interface.moc"
#endif // KIPI
