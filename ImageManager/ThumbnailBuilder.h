// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef THUMBNAILBUILDER_H
#define THUMBNAILBUILDER_H

#include "ImageClientInterface.h"
#include "enums.h"

#include <DB/ImageInfoPtr.h>
#include <DB/ImageScout.h>
#include <kpabase/FileNameList.h>

#include <QAtomicInt>
#include <QImage>

namespace MainWindow
{
class StatusBar;
}
namespace MainWindow
{
class Window;
}

class QTimer;

namespace ImageManager
{
class ThumbnailCache;

class ThumbnailBuilder : public QObject, public ImageManager::ImageClientInterface
{
    Q_OBJECT

public:
    static ThumbnailBuilder *instance();

    ~ThumbnailBuilder() override;
    void pixmapLoaded(ImageRequest *request, const QImage &image) override;
    void requestCanceled() override;

public Q_SLOTS:
    void buildAll(ThumbnailBuildStart when = ImageManager::StartDelayed);
    void buildMissing();
    void cancelRequests();
    void scheduleThumbnailBuild(const DB::FileNameList &list, ThumbnailBuildStart when);
    void buildOneThumbnail(const DB::ImageInfoPtr &fileName);
    void doThumbnailBuild();
    void save();

private:
    friend class MainWindow::Window;
    static ThumbnailBuilder *s_instance;
    ThumbnailBuilder(MainWindow::StatusBar *statusBar, QObject *parent, ThumbnailCache *thumbnailCache);
    void terminateScout();
    MainWindow::StatusBar *m_statusBar;
    ThumbnailCache *m_thumbnailCache;
    int m_count;
    int m_expectedThumbnails;
    bool m_isBuilding;
    QAtomicInt m_loadedCount;
    DB::ImageScoutQueue *m_preloadQueue;
    DB::ImageScout *m_scout;
    QTimer *m_startBuildTimer;
    DB::FileNameList m_thumbnailsToBuild;
};
}

#endif /* THUMBNAILBUILDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
