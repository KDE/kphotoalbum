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

#include "ImageFolder.h"
#include <klocale.h>
#include "DB/ImageDB.h"
#include "DB/ImageSearchInfo.h"
#include <kglobal.h>
#include <kiconloader.h>
#include "ThumbnailView/ThumbnailWidget.h"
#include "MainWindow/Window.h"

// PENDING(blackie) cleanup, we don't need from and to anymore
Browser::ImageFolder::ImageFolder( const DB::ImageSearchInfo& info, BrowserWidget* parent )
    :Folder( info, parent )
{
    DB::MediaCount count = DB::ImageDB::instance()->count( info );
    setCount( count );
}

QPixmap Browser::ImageFolder::pixmap()
{
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kphotoalbum" ), KIcon::Desktop, 22 );
}

QString Browser::ImageFolder::text() const
{
    return i18n( "Show Thumbnails" );
}



void Browser::ImageFolderAction::action( BrowserItemFactory* )
{
    MainWindow::Window::theMainWindow()->showThumbNails( DB::ImageDB::instance()->search( _info ) );

    if ( !_context.isNull() )
        ThumbnailView::ThumbnailWidget::theThumbnailView()->setCurrentItem( _context );
}

Browser::FolderAction* Browser::ImageFolder::action( bool /* ctrlDown */ )
{
    return new ImageFolderAction( _info, _browser );
}

Browser::ImageFolderAction::ImageFolderAction( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : FolderAction( info, browser ), _addExtraToBrowser( true )
{
}

Browser::ImageFolderAction::ImageFolderAction( const QString& context, BrowserWidget* browser )
    :FolderAction( DB::ImageSearchInfo(), browser ), _addExtraToBrowser(false), _context( context )
{
}

QString Browser::ImageFolder::imagesLabel() const
{
    return i18n( "1 image", "%n images", _count.images() );
}

QString Browser::ImageFolder::videosLabel() const
{
    return i18n( "1 video", "%n videos", _count.videos() );
}

