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

#include "ThumbnailBuilder.h"
#include "ImageManager/Manager.h"
#include "DB/ImageDB.h"
#include <klocale.h>
#include "Settings/SettingsData.h"
#include <qimage.h>
#include "DB/ImageInfo.h"

ThumbnailView::ThumbnailBuilder::ThumbnailBuilder( QWidget* parent, const char* name )
    :QProgressDialog( parent, name )
{
    _images = DB::ImageDB::instance()->images();
    setTotalSteps( _images.count() );
    setProgress( 0 );
    setLabelText( i18n("Generating thumbnails") );
    _index = 0;
    generateNext();
}

void ThumbnailView::ThumbnailBuilder::generateNext()
{
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(_images[_index]);
    ++_index;
    setProgress( _index );
    _infoMap.insert( info->fileName(), info );
    ImageManager::ImageRequest* request = new ImageManager::ImageRequest( info->fileName(),  QSize(256,256), info->angle(), this );
    request->setCache();
    request->setPriority();
    ImageManager::Manager::instance()->load( request );
}

void ThumbnailView::ThumbnailBuilder::pixmapLoaded( const QString& fileName, const QSize& /*size*/, const QSize& fullSize, int, const QImage&, bool /*loadedOK*/ )
{
    Q_ASSERT( _infoMap.contains( fileName ) );
    if ( fullSize.width() != -1 )
        _infoMap[fileName]->setSize( fullSize );
    if ( wasCanceled() )
        delete this;
    else if ( _index == _images.count() )
        delete this;
    else
        generateNext();
}

#include "ThumbnailBuilder.moc"
