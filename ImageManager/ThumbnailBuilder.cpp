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

#include "AsyncLoader.h"
#include "PreloadRequest.h"
#include "Logging.h"
#include "ThumbnailBuilder.h"
#include "ThumbnailCache.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfoPtr.h>
#include <DB/OptimizedFileList.h>
#include <MainWindow/StatusBar.h>
#include <ThumbnailView/CellGeometry.h>

#include <KLocalizedString>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QTimer>

ImageManager::ThumbnailBuilder* ImageManager::ThumbnailBuilder::s_instance = nullptr;

ImageManager::ThumbnailBuilder::ThumbnailBuilder( MainWindow::StatusBar* statusBar, QObject* parent )
    :QObject( parent ), 
     m_statusBar( statusBar ),
     m_count( 0 ),
     m_isBuilding( false ),
     m_loadedCount( 0 ),
     m_preloadQueue( nullptr ),
     m_scout( nullptr )
{
    connect(m_statusBar, &MainWindow::StatusBar::cancelRequest, this, &ThumbnailBuilder::cancelRequests);
    s_instance =  this;

    // Make sure that this is created early, in the main thread, so it
    // can receive signals.
    ThumbnailCache::instance();
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

void ImageManager::ThumbnailBuilder::terminateScout()
{
    if (m_scout) {
        delete m_scout;
        m_scout = nullptr;
    }
    if (m_preloadQueue) {
        delete m_preloadQueue;
        m_preloadQueue = nullptr;
    }
}

void ImageManager::ThumbnailBuilder::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& /*image*/)
{
    const DB::FileName fileName = request->databaseFileName();
    const QSize fullSize = request->fullSize();

    if ( fullSize.width() != -1 ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( fileName );
        info->setSize( fullSize );
    }
    m_loadedCount++;
    m_statusBar->setProgress( ++m_count );
    if ( m_count >= m_expectedThumbnails ) {
        terminateScout();
    }
}

void ImageManager::ThumbnailBuilder::buildAll( ThumbnailBuildStart when )
{
    QMessageBox msgBox;
    msgBox.setText(QString::fromLatin1("Buliding all thumbnails may take a long time."));
    msgBox.setInformativeText(QString::fromLatin1("Do you want to rebuild all of your thumbnails?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if ( ret == QMessageBox::Yes ) {
        ImageManager::ThumbnailCache::instance()->flush();
        scheduleThumbnailBuild( DB::ImageDB::instance()->images(), when );
    }
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

    DB::OptimizedFileList files(list);
    m_thumbnailsToBuild = files.optimizedDbFiles();
    m_startBuildTimer->start( when == StartNow ? 0 : 5000 );
}

void ImageManager::ThumbnailBuilder::buildOneThumbnail( const DB::ImageInfoPtr& info )
{
    ImageManager::ImageRequest* request
        = new ImageManager::PreloadRequest( info->fileName(),
                                          ThumbnailView::CellGeometry::preferredIconSize(), info->angle(),
                                          this );
    request->setIsThumbnailRequest(true);
    request->setPriority( ImageManager::BuildThumbnails );
    ImageManager::AsyncLoader::instance()->load( request );
}

void ImageManager::ThumbnailBuilder::doThumbnailBuild()
{
    m_isBuilding = true;
    int numberOfThumbnailsToBuild = 0;

    terminateScout();

    m_count = 0;
    m_loadedCount = 0;
    m_preloadQueue = new DB::ImageScoutQueue;
    for (const DB::FileName& fileName : m_thumbnailsToBuild ) {
        m_preloadQueue->enqueue(fileName);
    }
    qCDebug(ImageManagerLog) << "thumbnail builder starting scout";
    m_scout = new DB::ImageScout(*m_preloadQueue, m_loadedCount, 1);
    m_scout->setMaxSeekAhead(10);
    m_scout->setReadLimit(10 * 1048576);
    m_scout->start();
    m_statusBar->startProgress( i18n("Building thumbnails"), qMax( m_thumbnailsToBuild.size() - 1, 1 ) );
    // We'll update this later.  Meanwhile, we want to make sure that the scout
    // isn't prematurely terminated because the expected number of thumbnails
    // is less than (i. e. zero) the number of thumbnails actually built.
    m_expectedThumbnails = m_thumbnailsToBuild.size();
    for (const DB::FileName& fileName : m_thumbnailsToBuild ) {
        DB::ImageInfoPtr info = fileName.info();
        if ( ImageManager::AsyncLoader::instance()->isExiting() ) {
            cancelRequests();
            break;
        }
        if ( info->isNull())
        {
            m_loadedCount++;
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
    m_expectedThumbnails = numberOfThumbnailsToBuild;
    if (numberOfThumbnailsToBuild == 0) {
        m_statusBar->setProgressBarVisible(false);
        terminateScout();
    }
}

void ImageManager::ThumbnailBuilder::save()
{
    ImageManager::ThumbnailCache::instance()->save();
}

void ImageManager::ThumbnailBuilder::requestCanceled()
{
    m_statusBar->setProgress( ++m_count );
    m_loadedCount++;
    if ( m_count >= m_expectedThumbnails ) {
        terminateScout();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
