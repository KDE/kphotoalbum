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

#include <KLocale>
#include "ThumbnailBuilder.h"
#include "ImageManager/ThumbnailCache.h"
#include "MainWindow/StatusBar.h"
#include "CellGeometry.h"
#include "ImageManager/Manager.h"
#include "DB/ImageDB.h"
#include "DB/ResultId.h"

ThumbnailView::ThumbnailBuilder* ThumbnailView::ThumbnailBuilder::m_instance = 0;

ThumbnailView::ThumbnailBuilder::ThumbnailBuilder( MainWindow::StatusBar* statusBar, QObject* parent )
    :QObject( parent ), m_statusBar( statusBar ),  m_isBuilding( false )
{
    connect( m_statusBar, SIGNAL( cancelRequest() ), this, SLOT( cancelRequests() ) );
    m_instance =  this;
}

void ThumbnailView::ThumbnailBuilder::cancelRequests()
{
    ImageManager::Manager::instance()->stop( this, ImageManager::StopAll );
    m_isBuilding = false;
}

void ThumbnailView::ThumbnailBuilder::pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int, const QImage&, const bool loadedOK)
{
    Q_UNUSED(size)
    Q_UNUSED(loadedOK)
    if ( fullSize.width() != -1 ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( fileName, DB::AbsolutePath );
        info->setSize( fullSize );
    }
    m_statusBar->setProgress( ++m_count );
}

void ThumbnailView::ThumbnailBuilder::buildAll()
{
    const DB::Result images = DB::ImageDB::instance()->images();
    build( images.fetchInfos() );
}

ThumbnailView::ThumbnailBuilder* ThumbnailView::ThumbnailBuilder::instance()
{
    Q_ASSERT( m_instance );
    return m_instance;
}

void ThumbnailView::ThumbnailBuilder::buildMissing()
{
    const DB::Result images = DB::ImageDB::instance()->images();
    const QList<DB::ImageInfoPtr> list = images.fetchInfos();
    QList<DB::ImageInfoPtr> needed;
    Q_FOREACH( const DB::ImageInfoPtr& info, list ) {
        if ( ! ImageManager::ThumbnailCache::instance()->contains( info->fileName(DB::AbsolutePath) ) )
            needed.append( info );
    }
    build( needed );
}

void ThumbnailView::ThumbnailBuilder::build( const QList<DB::ImageInfoPtr>& list )
{
    if ( m_isBuilding )
        cancelRequests();

    if ( list.count() == 0 )
        return;

    m_isBuilding = true;
    m_statusBar->startProgress( i18n("Building thumbnails"), qMax( list.size() - 1, 1 ) );

    Q_FOREACH(const DB::ImageInfoPtr info, list) {
        ImageManager::ImageRequest* request
            = new ImageManager::ImageRequest( info->fileName(DB::AbsolutePath),
                                              CellGeometry::preferredIconSize(), info->angle(),
                                              this );
        request->setIsThumbnailRequest(true);
        request->setPriority( ImageManager::BuildThumbnails );
        ImageManager::Manager::instance()->load( request );
    }
    m_count = 0;
}

#include "ThumbnailBuilder.moc"
