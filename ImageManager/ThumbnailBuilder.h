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

#ifndef THUMBNAILBUILDER_H
#define THUMBNAILBUILDER_H

#include <QImage>
#include "ImageManager/ImageClientInterface.h"
#include "DB/ImageInfoPtr.h"
#include "enums.h"
#include <DB/FileNameList.h>
#include <QAtomicInt>
#include <DB/ImageScout.h>

namespace MainWindow { class StatusBar; }
namespace MainWindow { class Window; }

class QTimer;

namespace ImageManager
{

class ThumbnailBuilder :public QObject, public ImageManager::ImageClientInterface {
    Q_OBJECT

public:
    static ThumbnailBuilder* instance();

    void pixmapLoaded(ImageRequest* request, const QImage& image) override;
    void requestCanceled() override;

public slots:
    void buildAll(ThumbnailBuildStart when=ImageManager::StartDelayed );
    void buildMissing();
    void cancelRequests( );
    void scheduleThumbnailBuild( const DB::FileNameList& list, ThumbnailBuildStart when );
    void buildOneThumbnail( const DB::ImageInfoPtr& fileName );
    void doThumbnailBuild();
    void save();

private:
    friend class MainWindow::Window;
    static ThumbnailBuilder* s_instance;
    ThumbnailBuilder( MainWindow::StatusBar* statusBar, QObject* parent );
    void terminateScout();
    MainWindow::StatusBar* m_statusBar;
    int m_count;
    int m_expectedThumbnails;
    bool m_isBuilding;
    QAtomicInt m_loadedCount;
    DB::ImageScoutQueue *m_preloadQueue;
    DB::ImageScout *m_scout;
    QTimer* m_startBuildTimer;
    DB::FileNameList m_thumbnailsToBuild;
};

}

#endif /* THUMBNAILBUILDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
