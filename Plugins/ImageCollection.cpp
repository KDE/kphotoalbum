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
#include "Plugins/ImageCollection.h"
#include "mainview.h"
#include "imagedb.h"
#include "options.h"
#include <qfileinfo.h>
#include "imageinfolist.h"
#include "imageinfo.h"

Plugins::ImageCollection::ImageCollection( Type tp )
    : _tp( tp )
{
}

QString Plugins::ImageCollection::name()
{
    // This doesn't really make much sence for selection and current album.
    return QString::null;
}

QString Plugins::ImageCollection::comment()
{
    return QString::null;
}

KURL::List Plugins::ImageCollection::images()
{
    switch ( _tp ) {
    case CurrentAlbum:
        return stringListToUrlList( ImageDB::instance()->currentScope( false ) );

    case CurrentSelection:
        return stringListToUrlList( MainView::theMainView()->selected() );

    case SubClass:
        qFatal( "The subclass should implement images()" );
        return KURL::List();
    }
    return KURL::List();
}

KURL::List Plugins::ImageCollection::imageListToUrlList( const ImageInfoList& imageList )
{
    KURL::List urlList;
    for( ImageInfoListConstIterator it = imageList.constBegin(); it != imageList.constEnd(); ++it ) {
        KURL url;
        url.setPath( (*it)->fileName() );
        urlList.append( url );
    }
    return urlList;
}

KURL::List Plugins::ImageCollection::stringListToUrlList( const QStringList& list )
{
    KURL::List urlList;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        KURL url;
        url.setPath( *it );
        urlList.append( url );
    }
    return urlList;
}

KURL Plugins::ImageCollection::path()
{
    return commonRoot();
}

KURL Plugins::ImageCollection::commonRoot()
{
    QString imgRoot = Options::instance()->imageDirectory();
    const KURL::List imgs = images();
    if ( imgs.count() == 0 )
        return imgRoot;

    QStringList res = QStringList::split( QString::fromLatin1( "/" ), QFileInfo( imgs[0].path() ).dirPath(true), true );

    for( KURL::List::ConstIterator it = imgs.begin(); it != imgs.end(); ++it ) {
        QStringList newRes;

        QStringList path = QStringList::split( QString::fromLatin1( "/" ), QFileInfo( (*it).path() ).dirPath( true ), true );
        uint i = 0;
        for ( ; i < QMIN( path.size(), res.size() ); ++i ) {
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

    KURL url;
    url.setPath( result );
    return url;
}

KURL Plugins::ImageCollection::uploadPath()
{
    return commonRoot();
}

KURL Plugins::ImageCollection::uploadRoot()
{
    KURL url;
    url.setPath( Options::instance()->imageDirectory() );
    return url;
}

#endif // KIPI
