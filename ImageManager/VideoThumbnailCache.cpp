// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoThumbnailCache.h"
#include "qglobal.h"
#include <QCryptographicHash>

#include <kpabase/Logging.h>

namespace
{

constexpr int MAX_FRAMES = 10;
/**
 * @brief LRU_SIZE is the size of the in-memory cache of animated thumbnails.
 * I.e. a LRU_SIZE of 1 means that 1 animated thumbnail with MAX_FRAMES frames is kept in memory.
 */
constexpr size_t LRU_SIZE = 1;
}

ImageManager::VideoThumbnailCache::VideoThumbnailCache(const QString &baseDirectory, QObject *parent)
    : QObject { parent }
    , m_baseDir(baseDirectory)
    , m_memcache(LRU_SIZE)
{
    qCInfo(ImageManagerLog) << "Using video thumbnail directory" << m_baseDir.absolutePath();
    if (!m_baseDir.exists()) {
        if (!QDir().mkpath(m_baseDir.path())) {
            qCWarning(ImageManagerLog, "Failed to create video thumbnail cache directory!");
        }
    }
}

QVector<QImage> ImageManager::VideoThumbnailCache::lookup(const DB::FileName &name) const
{
    QVector<QImage> frames { MAX_FRAMES };
    for (int i = 0; i < 10; ++i) {
        const DB::FileName thumbnailFile = frameName(name, i);
        if (!thumbnailFile.exists())
            return {};

        QImage frame { thumbnailFile.absolute() };
        if (frame.isNull())
            return {};

        frames[i] = frame;
    }
    return frames;
}

bool ImageManager::VideoThumbnailCache::contains(const DB::FileName &name) const
{
    for (int i = 0; i < 10; ++i) {
        const DB::FileName thumbnailFile = frameName(name, i);
        if (!thumbnailFile.exists())
            return false;
    }
    return true;
}

void ImageManager::VideoThumbnailCache::removeThumbnail(const DB::FileName &name)
{
}

void ImageManager::VideoThumbnailCache::removeThumbnails(const DB::FileNameList &names)
{
    for (const auto &name : names) {
        removeThumbnail(name);
    }
}

QString ImageManager::VideoThumbnailCache::nameHash(const DB::FileName &videoName) const
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(videoName.absolute().toUtf8());
    return QString::fromUtf8(md5.result().toHex());
}

DB::FileName ImageManager::VideoThumbnailCache::frameName(const DB::FileName &videoName, int frameNumber) const
{
    Q_ASSERT_X(0 <= frameNumber && frameNumber < MAX_FRAMES, "VideoThumbnailCache::frameName", "Video thumbnail frame index out of bounds!");
    const QString frameName = QString::fromUtf8("%1-%2").arg(nameHash(videoName), frameNumber);
    return DB::FileName::fromAbsolutePath(m_baseDir.absoluteFilePath(frameName));
}

QString ImageManager::defaultVideoThumbnailDirectory()
{
    return QString::fromLatin1(".videoThumbnails/");
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VideoThumbnailCache.cpp"
