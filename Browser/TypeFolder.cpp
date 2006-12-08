/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include "DB/ImageDB.h"
#include <klocale.h>
#include "TypeFolderAction.h"

Browser::TypeFolder::TypeFolder( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* parent )
    :Folder( info, parent ), _category ( category )
{
    QMap<QString, uint> images = DB::ImageDB::instance()->classify( _info, _category->name(), DB::Image );
    QMap<QString, uint> videos = DB::ImageDB::instance()->classify( _info, _category->name(), DB::Video );
    DB::MediaCount count( images.count(), videos.count() );
    setCount( count );
    if ( count.total() <= 1 )
        setEnabled( false );
}

QPixmap Browser::TypeFolder::pixmap()
{
    return _category->icon();
}

QString Browser::TypeFolder::text() const
{
    return _category->text();
}

Browser::FolderAction* Browser::TypeFolder::action( bool /* ctrlDown */ )
{
    return new TypeFolderAction( _category, _info, _browser );
}

QString Browser::TypeFolder::imagesLabel() const
{
    return i18n("1 Category", "%n Categories", _count.images());
}

QString Browser::TypeFolder::videosLabel() const
{
    return i18n("1 Category", "%n Categories", _count.videos());
}

