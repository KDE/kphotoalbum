/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "ThumbnailBuilder.h"
#include <KLocalizedString>
#include "ImageManager/ThumbnailCache.h"
#include "MainWindow/StatusBar.h"
#include "ThumbnailView/CellGeometry.h"
#include "ImageManager/AsyncLoader.h"
#include "DB/ImageDB.h"
#include "PreloadRequest.h"
#include <QTimer>
#include <DB/ImageInfoPtr.h>

ImageManager::ThumbnailBuilder* ImageManager::ThumbnailBuilder::s_instance = nullptr;

ImageManager::ThumbnailBuilder::ThumbnailBuilder( MainWindow::StatusBar* statusBar, QObject* parent )
    :QObject( parent ), m_statusBar( statusBar ),  m_isBuilding( false )
{
    connect(m_statusBar, &MainWindow::StatusBar::cancelRequest, this, &ThumbnailBuilder::cancelRequests);
    s_instance =  this;

    m_startBuildTimer = new QTimer(this);
    m_startBuildTimer->setSingleShot(true);
    connect(m_startBuildTimer, &QTimer::timeout, this, &ThumbnailBuilder::doThumbnailBuild);
}

void ImageManager::ThumbnailBuilder::cancelRequests()
{
    ImageManager::AsyncLoader::instance()->stop( this, ImageManager::StopAll );
    m_isBuilding = false;
    m_statusBar->setProgressBarVisible(false);
    m_startBuildTimer->stop();
}

void ImageManager::ThumbnailBuilder::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& /*image*/)
{
    const DB::FileName fileName = request->databaseFileName();
    const QSize fullSize = request->fullSize();

    if ( fullSize.width() != -1 ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( fileName );
        info->setSize( fullSize );
    }
    m_statusBar->setProgress( ++m_count );
}

void ImageManager::ThumbnailBuilder::buildAll( ThumbnailBuildStart when )
{
    ImageManager::ThumbnailCache::instance()->flush();
    scheduleThumbnailBuild( DB::ImageDB::instance()->images(), when );
}

ImageManager::ThumbnailBuilder* ImageManager::ThumbnailBuilder::instance()
{
    Q_ASSERT( s_instance );
    return s_instance;
}

void ImageManager::ThumbnailBuilder::buildMissing()
{
    const DB::FileNameList images = DB::ImageDB::instance()->images();
    DB::FileNameList needed;
    for ( const DB::FileName& fileName : images ) {
        if ( ! ImageManager::ThumbnailCache::instance()->contains( fileName ) )
            needed.append( fileName );
    }
    scheduleThumbnailBuild( needed, StartDelayed );
}

void ImageManager::ThumbnailBuilder::scheduleThumbnailBuild( const DB::FileNameList& list, ThumbnailBuildStart when )
{
    if ( list.count() == 0 )
        return;

    if ( m_isBuilding )
        cancelRequests();

    m_startBuildTimer->start( when == StartNow ? 0 : 5000 );
    m_thumbnailsToBuild = list;
}

void ImageManager::ThumbnailBuilder::doThumbnailBuild()
{
    m_isBuilding = true;
    int numberOfThumbnailsToBuild = 0;

    m_count = 0;
    m_statusBar->startProgress( i18n("Building thumbnails"), qMax( m_thumbnailsToBuild.size() - 1, 1 ) );
    for (const DB::FileName& fileName : m_thumbnailsToBuild ) {
        DB::ImageInfoPtr info = fileName.info();
        if ( info->isNull())
        {
            m_count++;
            continue;
        }

        ImageManager::ImageRequest* request
            = new ImageManager::PreloadRequest( fileName,
                                              ThumbnailView::CellGeometry::preferredIconSize(), info->angle(),
                                              this );
        request->setIsThumbnailRequest(true);
        request->setPriority( ImageManager::BuildThumbnails );
        if (ImageManager::AsyncLoader::instance()->load( request ))
            ++numberOfThumbnailsToBuild;
    }
    if (numberOfThumbnailsToBuild == 0)
        m_statusBar->setProgressBarVisible(false);
}

void ImageManager::ThumbnailBuilder::requestCanceled()
{
    m_statusBar->setProgress( ++m_count );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
