/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
