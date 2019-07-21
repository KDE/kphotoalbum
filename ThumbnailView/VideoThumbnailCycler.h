/* Copyright 2012  Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VIDEOTHUMBNAILCYCLER_H
#define VIDEOTHUMBNAILCYCLER_H

#include <DB/FileName.h>

#include <QObject>

class QTimer;
class QImage;

namespace DB
{
class FileName;
}
namespace ImageManager
{
class VideoThumbnails;
}

namespace ThumbnailView
{

class ThumbnailModel;

/**
  \brief Class which is responsible for cycling the video thumbnails in the thumbnail viewer
  \see \ref videothumbnails
*/
class VideoThumbnailCycler : public QObject
{
    Q_OBJECT
public:
    explicit VideoThumbnailCycler(ThumbnailModel *model, QObject *parent = nullptr);
    static VideoThumbnailCycler *instance();
    void setActive(const DB::FileName &id);
    void stopCycle();

private slots:
    void gotFrame(const QImage &image);

private:
    void resetPreviousThumbail();
    bool isVideo(const DB::FileName &fileName) const;
    void startCycle();

    static VideoThumbnailCycler *s_instance;
    DB::FileName m_fileName;
    QTimer *m_timer;
    ImageManager::VideoThumbnails *m_thumbnails;
    ThumbnailModel *m_model;
};

}
#endif // VIDEOTHUMBNAILCYCLER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
