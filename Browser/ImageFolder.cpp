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

#include "ImageFolder.h"
#include <klocale.h>
#include "imagedb.h"
#include "imagesearchinfo.h"
#include "options.h"
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "ThumbnailView/ThumbnailView.h"
#include "mainview.h"

// PENDING(blackie) cleanup, we don't need from and to anymore
Browser::ImageFolder::ImageFolder( const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent )
{
    int count = ImageDB::instance()->count( info );
    setCount( count );
}

QPixmap Browser::ImageFolder::pixmap()
{
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kphotoalbum" ), KIcon::Desktop, 22 );
}

QString Browser::ImageFolder::text() const
{
    return i18n( "View Images" );
}



void Browser::ImageFolderAction::action( BrowserItemFactory* )
{
    MainView::theMainView()->showThumbNails( ImageDB::instance()->search( _info ) );

    if ( !_context.isNull() )
        ThumbnailView::ThumbnailView::theThumbnailView()->setCurrentItem( _context );
}

Browser::FolderAction* Browser::ImageFolder::action( bool /* ctrlDown */ )
{
    return new ImageFolderAction( _info, _browser );
}

Browser::ImageFolderAction::ImageFolderAction( const ImageSearchInfo& info, Browser* browser )
    : FolderAction( info, browser ), _addExtraToBrowser( true )
{
}

Browser::ImageFolderAction::ImageFolderAction( const QString& context, Browser* browser )
    :FolderAction( ImageSearchInfo(), browser ), _addExtraToBrowser(false), _context( context )
{
}

QString Browser::ImageFolder::countLabel() const
{
    return i18n("1 image", "%n images", _count );
}

