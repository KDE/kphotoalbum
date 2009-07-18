/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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

#include <KLocale>
#include "ThumbnailBuilder.h"
#include "ImageManager/Manager.h"
#include "DB/ImageDB.h"
#include "DB/ResultId.h"

ThumbnailView::ThumbnailBuilder::ThumbnailBuilder( QWidget* parent )
    :QProgressDialog( parent )
{
    const DB::Result images = DB::ImageDB::instance()->images();
    setMaximum( qMax( images.size() - 1, 0 ) );
    setLabelText( i18n("Generating thumbnails") );

    connect( this, SIGNAL( canceled() ), this, SLOT( slotCancelRequests() ) );

    Q_FOREACH(const DB::ImageInfoPtr info, images.fetchInfos()) {
        ImageManager::ImageRequest* request
            = new ImageManager::ImageRequest( info->fileName(DB::AbsolutePath),
                                              QSize(256,256), info->angle(),
                                              this );
        request->setPriority( ImageManager::BuildThumbnails );
        ImageManager::Manager::instance()->load( request );
    }
}

void ThumbnailView::ThumbnailBuilder::slotCancelRequests()
{
    ImageManager::Manager::instance()->stop( this, ImageManager::StopAll );
    setValue( maximum() );
}

void ThumbnailView::ThumbnailBuilder::pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int, const QImage&, const bool loadedOK)
{
    Q_UNUSED(size)
    Q_UNUSED(loadedOK)
    if ( fullSize.width() != -1 ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( fileName, DB::AbsolutePath );
        info->setSize( fullSize );
    }
    setValue( value() + 1 );
}

#include "ThumbnailBuilder.moc"
