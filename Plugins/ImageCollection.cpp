/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImageCollection.h"
#include "MainWindow/Window.h"
#include "DB/ImageDB.h"
#include "Settings/SettingsData.h"
#include <qfileinfo.h>
#include "DB/ImageInfoList.h"
#include "DB/ImageInfo.h"

Plugins::ImageCollection::ImageCollection( Type tp )
    : m_type( tp )
{
}

QString Plugins::ImageCollection::name()
{
    QString res;
    switch ( m_type ) {
    case CurrentAlbum:
        res = MainWindow::Window::theMainWindow()->currentContext().toString();
        break;
    case CurrentSelection:
        res = MainWindow::Window::theMainWindow()->currentContext().toString();
        if (res.isEmpty()) {
            res = i18nc("As in 'an unknown set of images, created from the selection'.","Unknown (Selection)");
        }
        else {
            res += i18nc("As in 'A selection of [a generated context description]'"," (Selection)");
        }
        break;
    case SubClass:
        qWarning("Subclass of ImageCollection should overwrite ImageCollection::name()");
        res = i18nc("A set of images with no description.","Unknown");
        break;
    }
    if (res.isEmpty()) {
        // at least html export plugin needs a none-empty name:
        res = i18nc("The 'name' of an unnamed image collection.","None");
    }
    return res;
}

QString Plugins::ImageCollection::comment()
{
    return QString();
}

KUrl::List Plugins::ImageCollection::images()
{
    switch ( m_type ) {
    case CurrentAlbum:
        return stringListToUrlList( DB::ImageDB::instance()->currentScope( false ).toStringList(DB::AbsolutePath));

    case CurrentSelection:
        return stringListToUrlList( MainWindow::Window::theMainWindow()->selected(ThumbnailView::NoExpandCollapsedStacks).toStringList(DB::AbsolutePath));

    case SubClass:
        qFatal( "The subclass should implement images()" );
        return KUrl::List();
    }
    return KUrl::List();
}

KUrl::List Plugins::ImageCollection::imageListToUrlList( const DB::ImageInfoList& imageList )
{
    KUrl::List urlList;
    for( DB::ImageInfoListConstIterator it = imageList.constBegin(); it != imageList.constEnd(); ++it ) {
        KUrl url;
        url.setPath( (*it)->fileName().absolute() );
        urlList.append( url );
    }
    return urlList;
}

KUrl::List Plugins::ImageCollection::stringListToUrlList( const QStringList& list )
{
    KUrl::List urlList;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        KUrl url;
        url.setPath( *it );
        urlList.append( url );
    }
    return urlList;
}

KUrl Plugins::ImageCollection::path()
{
    return commonRoot();
}

KUrl Plugins::ImageCollection::commonRoot()
{
    QString imgRoot = Settings::SettingsData::instance()->imageDirectory();
    const KUrl::List imgs = images();
    if ( imgs.count() == 0 )
        return imgRoot;

    QStringList res = QFileInfo( imgs[0].path() ).absolutePath().split( QLatin1String( "/" ) );

    for( KUrl::List::ConstIterator it = imgs.begin(); it != imgs.end(); ++it ) {
        QStringList newRes;

        QStringList path = QFileInfo( (*it).path() ).absolutePath().split( QLatin1String( "/" ) );
        int i = 0;
        for ( ; i < qMin( path.size(), res.size() ); ++i ) {
            if ( path[i] == res[i] )
                newRes.append( res[i] );
            else
                break;
        }
        res = newRes;
    }

    QString result = res.join( QString::fromLatin1( "/" ) );
    if ( result.left( imgRoot.length() ) != imgRoot ) {
        result = imgRoot;
    }

    KUrl url;
    url.setPath( result );
    return url;
}

KUrl Plugins::ImageCollection::uploadPath()
{
    return commonRoot();
}

KUrl Plugins::ImageCollection::uploadRoot()
{
    KUrl url;
    url.setPath( Settings::SettingsData::instance()->imageDirectory() );
    return url;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
