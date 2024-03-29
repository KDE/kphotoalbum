// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ThumbnailBuilder.h"

#include "AsyncLoader.h"
#include "PreloadRequest.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfoPtr.h>
#include <DB/OptimizedFileList.h>
#include <MainWindow/StatusBar.h>
#include <ThumbnailView/CellGeometry.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <KLocalizedString>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QTimer>

namespace
{
/**
 * @brief Thumbnail size for storage.
 * @see ThumbnailView::CellGeometry::preferredIconSize()
 * @return
 */
QSize preferredThumbnailSize()
{
    int width = Settings::SettingsData::instance()->thumbnailSize();
    return QSize(width, width);
}
}

ImageManager::ThumbnailBuilder *ImageManager::ThumbnailBuilder::s_instance = nullptr;

ImageManager::ThumbnailBuilder::ThumbnailBuilder(MainWindow::StatusBar *statusBar, QObject *parent, ThumbnailCache *thumbnailCache)
    : QObject(parent)
    , m_statusBar(statusBar)
    , m_thumbnailCache(thumbnailCache)
    , m_count(0)
    , m_expectedThumbnails(0)
    , m_isBuilding(false)
    , m_loadedCount(0)
    , m_preloadQueue(nullptr)
    , m_scout(nullptr)
{
    connect(m_statusBar, &MainWindow::StatusBar::cancelRequest, this, &ThumbnailBuilder::cancelRequests);
    s_instance = this;

    m_startBuildTimer = new QTimer(this);
    m_startBuildTimer->setSingleShot(true);
    connect(m_startBuildTimer, &QTimer::timeout, this, &ThumbnailBuilder::doThumbnailBuild);
}

void ImageManager::ThumbnailBuilder::cancelRequests()
{
    ImageManager::AsyncLoader::instance()->stop(this, ImageManager::StopAll);
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

void ImageManager::ThumbnailBuilder::pixmapLoaded(ImageManager::ImageRequest *request, const QImage & /*image*/)
{
    const DB::FileName fileName = request->databaseFileName();
    const QSize fullSize = request->fullSize();
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);

    // We probably shouldn't do this at all, since the "full size"
    // of the request could be the size of the embedded thumbnail
    // or even a scaled-down such.  But if this hasn't been
    // set orrectly earlier, we have nothing else to go on.
    if (fullSize.width() != -1 && info->size().width() == -1) {
        info->setSize(fullSize);
    }
    m_loadedCount++;
    m_statusBar->setProgress(++m_count);
    if (m_count >= m_expectedThumbnails) {
        terminateScout();
    }
}

void ImageManager::ThumbnailBuilder::buildAll(ThumbnailBuildStart when)
{
    QMessageBox msgBox;
    msgBox.setText(i18n("Building all thumbnails may take a long time."));
    msgBox.setInformativeText(i18n("Do you want to rebuild all of your thumbnails?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes) {
        m_thumbnailCache->flush();
        scheduleThumbnailBuild(DB::ImageDB::instance()->files(), when);
    }
}

ImageManager::ThumbnailBuilder *ImageManager::ThumbnailBuilder::instance()
{
    Q_ASSERT(s_instance);
    return s_instance;
}

ImageManager::ThumbnailBuilder::~ThumbnailBuilder()
{
    terminateScout();
}

void ImageManager::ThumbnailBuilder::buildMissing()
{
    const DB::FileNameList images = DB::ImageDB::instance()->files();
    DB::FileNameList needed;
    for (const DB::FileName &fileName : images) {
        if (!m_thumbnailCache->contains(fileName))
            needed.append(fileName);
    }
    scheduleThumbnailBuild(needed, StartDelayed);
}

void ImageManager::ThumbnailBuilder::scheduleThumbnailBuild(const DB::FileNameList &list, ThumbnailBuildStart when)
{
    if (list.count() == 0)
        return;

    if (m_isBuilding)
        cancelRequests();

    DB::OptimizedFileList files(list);
    m_thumbnailsToBuild = files.optimizedDbFiles();
    m_startBuildTimer->start(when == StartNow ? 0 : 5000);
}

void ImageManager::ThumbnailBuilder::buildOneThumbnail(const DB::ImageInfoPtr &info)
{
    ImageManager::ImageRequest *request
        = new ImageManager::PreloadRequest(info->fileName(),
                                           preferredThumbnailSize(), info->angle(),
                                           this, m_thumbnailCache);
    request->setIsThumbnailRequest(true);
    request->setPriority(ImageManager::BuildThumbnails);
    ImageManager::AsyncLoader::instance()->load(request);
}

void ImageManager::ThumbnailBuilder::doThumbnailBuild()
{
    m_isBuilding = true;
    int numberOfThumbnailsToBuild = 0;

    terminateScout();

    m_count = 0;
    m_loadedCount = 0;
    m_preloadQueue = new DB::ImageScoutQueue;
    for (const DB::FileName &fileName : m_thumbnailsToBuild) {
        m_preloadQueue->enqueue(fileName);
    }
    qCDebug(ImageManagerLog) << "thumbnail builder starting scout";
    m_scout = new DB::ImageScout(*m_preloadQueue, m_loadedCount, Settings::SettingsData::instance()->getThumbnailPreloadThreadCount());
    m_scout->setMaxSeekAhead(10);
    m_scout->setReadLimit(10 * 1048576);
    m_scout->start();
    m_statusBar->startProgress(i18n("Building thumbnails"), qMax(m_thumbnailsToBuild.size() - 1, 1));
    // We'll update this later.  Meanwhile, we want to make sure that the scout
    // isn't prematurely terminated because the expected number of thumbnails
    // is less than (i. e. zero) the number of thumbnails actually built.
    m_expectedThumbnails = m_thumbnailsToBuild.size();
    for (const DB::FileName &fileName : m_thumbnailsToBuild) {
        const auto info = DB::ImageDB::instance()->info(fileName);
        if (ImageManager::AsyncLoader::instance()->isExiting()) {
            cancelRequests();
            break;
        }
        if (info->isNull()) {
            m_loadedCount++;
            m_count++;
            continue;
        }

        ImageManager::ImageRequest *request
            = new ImageManager::PreloadRequest(fileName,
                                               preferredThumbnailSize(), info->angle(),
                                               this, m_thumbnailCache);
        request->setIsThumbnailRequest(true);
        request->setPriority(ImageManager::BuildThumbnails);
        if (ImageManager::AsyncLoader::instance()->load(request))
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
    m_thumbnailCache->save();
}

void ImageManager::ThumbnailBuilder::requestCanceled()
{
    m_statusBar->setProgress(++m_count);
    m_loadedCount++;
    if (m_count >= m_expectedThumbnails) {
        terminateScout();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ThumbnailBuilder.cpp"
