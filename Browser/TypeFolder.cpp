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

#include "TypeFolder.h"
#include "Settings/SettingsData.h"
#include "DB/ImageDB.h"
#include "ContentFolder.h"
#include <klocale.h>
#include "BrowserItemFactory.h"
#include "DB/CategoryCollection.h"

Browser::TypeFolder::TypeFolder( const QString& category, const DB::ImageSearchInfo& info, BrowserWidget* parent )
    :Folder( info, parent ), _category ( category )
{
    QMap<QString, int> images = DB::ImageDB::instance()->classify( _info, _category, DB::Image );
    QMap<QString, int> movies = DB::ImageDB::instance()->classify( _info, _category, DB::Movie );
    DB::MediaCount count( images.count(), movies.count() );
    setCount( count );
    if ( count.total() <= 1 )
        setEnabled( false );
}

QPixmap Browser::TypeFolder::pixmap()
{
    return DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->icon();
}

QString Browser::TypeFolder::text() const
{
    return DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->text();
}

Browser::FolderAction* Browser::TypeFolder::action( bool /* ctrlDown */ )
{
    return new TypeFolderAction( _category, _info, _browser );
}

Browser::TypeFolderAction::TypeFolderAction( const QString& category, const DB::ImageSearchInfo& info,
                                    BrowserWidget* browser )
    :FolderAction( info, browser ), _category( category )
{
}

void Browser::TypeFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();


    QMap<QString, int> images = DB::ImageDB::instance()->classify( _info, _category, DB::Image );
    QMap<QString, int> movies = DB::ImageDB::instance()->classify( _info, _category, DB::Movie );

    QStringList keys = images.keys() + movies.keys();
    qHeapSort(keys);
    QString prev;

    for( QStringList::ConstIterator it = keys.begin(); it != keys.end(); ++it ) {
        if ( *it != prev && *it != DB::ImageDB::NONE() ) {
            factory->createItem( new ContentFolder( _category, *it, DB::MediaCount( images[*it], movies[*it] ), _info, _browser ) );
            prev = *it;
        }
    }

    // Add the none option to the end
    int imageCount = images[DB::ImageDB::NONE()];
    int movieCount = movies[DB::ImageDB::NONE()];
    if ( imageCount + movieCount != 0 )
        factory->createItem( new ContentFolder( _category, DB::ImageDB::NONE(), DB::MediaCount( imageCount, movieCount ),
                                                _info, _browser ) );
}

QString Browser::TypeFolderAction::title() const
{
    return DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->text();
}

QString Browser::TypeFolderAction::category() const
{
    return _category;
}

QString Browser::TypeFolder::imagesLabel() const
{
    return i18n("1 Category", "%n Categories", _count.images());
}

QString Browser::TypeFolder::moviesLabel() const
{
    return i18n("1 Category", "%n Categories", _count.movies());
}

bool Browser::TypeFolderAction::contentView() const
{
    return ( !DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->isSpecialCategory() );
}



