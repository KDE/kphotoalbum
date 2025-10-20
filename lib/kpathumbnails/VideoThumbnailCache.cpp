// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoThumbnailCache.h"
#include <QCryptographicHash>
#include <kpabase/FileUtil.h>
#include <kpabase/ImageUtil.h>
#include <kpabase/Logging.h>
#include <qglobal.h>

namespace
{

constexpr int MAX_FRAMES = 10;
/**
 * @brief LRU_SIZE is the size of the in-memory cache of animated thumbnails.
 * I.e. a LRU_SIZE of 1 means that 1 animated thumbnail with MAX_FRAMES frames is kept in memory.
 */
constexpr size_t LRU_SIZE = 1;

constexpr QFileDevice::Permissions FILE_PERMISSIONS { QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther };
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

const QVector<QImage> ImageManager::VideoThumbnailCache::lookup(const DB::FileName &name, LookupMode mode) const
{
    const auto cacheName = nameHash(name);
    if (m_memcache.contains(cacheName))
        return *m_memcache.object(cacheName);

    qCDebug(ImageManagerLog) << "Video thumbnail frames for" << name.relative() << "was not cached.";
    std::unique_ptr<QVector<QImage>> frames = std::make_unique<QVector<QImage>>(MAX_FRAMES);
    for (int i = 0; i < 10; ++i) {
        const DB::FileName thumbnailFile = frameName(name, i);
        qCDebug(ImageManagerLog) << "Thumbnail file" << thumbnailFile.relative() << "exists:" << thumbnailFile.exists();
        if (!thumbnailFile.exists()) {
            if (mode == LookupMode::Complete)
                return {};
            else
                continue;
        }

        QImage frame { thumbnailFile.absolute() };
        if (frame.isNull()) {
            qCDebug(ImageManagerLog) << "Video thumbnail frame" << i << "for" << name.relative() << "is on disk but empty.";
            if (mode == LookupMode::Complete)
                return {};
            else
                continue;
        }

        qCDebug(ImageManagerLog) << "Video thumbnail frame" << i << "for" << name.relative() << "is on disk.";
        (*frames)[i] = frame;
    }

    qCDebug(ImageManagerLog) << "Video thumbnail frames for" << name.relative() << "loaded.";
    auto *framesPtr = frames.release();
    m_memcache.insert(cacheName, framesPtr);
    return *framesPtr;
}

QImage ImageManager::VideoThumbnailCache::lookup(const DB::FileName &name, int frameNumber) const
{
    Q_ASSERT_X(0 <= frameNumber && frameNumber < MAX_FRAMES, "VideoThumbnailCache::lookup", "Video thumbnail frame index out of bounds!");

    const auto cacheName = nameHash(name);
    if (m_memcache.contains(cacheName)) {
        qCDebug(ImageManagerLog) << "Video thumbnail frame" << frameNumber << "for" << name.relative() << "is cached.";
        return m_memcache.object(cacheName)->at(frameNumber);
    }

    qCDebug(ImageManagerLog) << "Video thumbnail frame" << frameNumber << "for" << name.relative() << "was not cached.";
    // we need to load all frames into the cache eventually, so let's just trigger that now:
    return lookup(name).value(frameNumber);
}

QImage ImageManager::VideoThumbnailCache::lookupStillFrame(const DB::FileName &name) const
{
    const DB::FileName thumbnailFile = stillFrameName(name);
    qCDebug(ImageManagerLog) << "Thumbnail file" << thumbnailFile.relative() << "exists:" << thumbnailFile.exists();
    if (!thumbnailFile.exists())
        return {};

    return QImage { thumbnailFile.absolute() };
}

bool ImageManager::VideoThumbnailCache::contains(const DB::FileName &name) const
{
    const auto cacheName = nameHash(name);
    if (m_memcache.contains(cacheName))
        return true;

    for (int i = 0; i < MAX_FRAMES; ++i) {
        const DB::FileName thumbnailFile = frameName(name, i);
        if (!thumbnailFile.exists())
            return false;
    }
    return true;
}

bool ImageManager::VideoThumbnailCache::contains(const DB::FileName &name, int frameNumber) const
{
    const auto cacheName = nameHash(name);
    if (m_memcache.contains(cacheName))
        return true;

    const DB::FileName thumbnailFile = frameName(name, frameNumber);
    return thumbnailFile.exists();
}

void ImageManager::VideoThumbnailCache::insertThumbnail(const DB::FileName &name, int frameNumber, const QImage &image)
{
    if (0 > frameNumber || frameNumber >= MAX_FRAMES)
        return;

    if (image.isNull())
        return;

    qCDebug(ImageManagerLog) << "Inserting video thumbnail still frame for" << name.relative() << ":" << frameName(name, frameNumber).relative();
    Utilities::saveImage(frameName(name, frameNumber), image, "JPEG");
    if (frameNumber == 0)
        Utilities::copyOrOverwrite(frameName(name, frameNumber).absolute(), stillFrameName(name).absolute());
    const auto cacheName = nameHash(name);
    // purge stale data:
    m_memcache.remove(cacheName);
    if (contains(name))
        Q_EMIT frameUpdated(name, frameNumber);
}

void ImageManager::VideoThumbnailCache::blockThumbnail(const DB::FileName &name, int frameNumber)
{
    // Create empty file to avoid that we recheck at next start up.
    QFile file(frameName(name, frameNumber).absolute());
    if (file.open(QFile::WriteOnly)) {
        file.setPermissions(FILE_PERMISSIONS);
        file.close();
    }
}

void ImageManager::VideoThumbnailCache::removeThumbnail(const DB::FileName &name)
{
    for (int i = 0; i < 10; ++i) {
        const DB::FileName thumbnailFile = frameName(name, i);
        if (thumbnailFile.exists())
            QDir().remove(thumbnailFile.absolute());
    }
    const auto stillFile = stillFrameName(name);
    if (stillFile.exists())
        QDir().remove(stillFile.absolute());
}

void ImageManager::VideoThumbnailCache::removeThumbnails(const DB::FileNameList &names)
{
    for (const auto &name : names) {
        removeThumbnail(name);
    }
}

int ImageManager::VideoThumbnailCache::numberOfFrames() const
{
    return MAX_FRAMES;
}

QString ImageManager::VideoThumbnailCache::nameHash(const DB::FileName &videoName) const
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(videoName.absolute().toUtf8());
    return QString::fromUtf8(md5.result().toHex());
}

DB::FileName ImageManager::VideoThumbnailCache::stillFrameName(const DB::FileName &videoName) const
{
    // the still frame used for the static thumbnail has no trailing frame number (e.g. '-1')
    return DB::FileName::fromAbsolutePath(m_baseDir.absoluteFilePath(nameHash(videoName)));
}

DB::FileName ImageManager::VideoThumbnailCache::frameName(const DB::FileName &videoName, int frameNumber) const
{
    Q_ASSERT_X(0 <= frameNumber && frameNumber < MAX_FRAMES, "VideoThumbnailCache::frameName", "Video thumbnail frame index out of bounds!");
    const QString frameName = QString::fromUtf8("%1-%2").arg(nameHash(videoName)).arg(frameNumber);
    return DB::FileName::fromAbsolutePath(m_baseDir.absoluteFilePath(frameName));
}

int ImageManager::VideoThumbnailCache::stillFrameIndex(const DB::FileName &name) const
{
    const auto stillFrame = lookupStillFrame(name);
    if (stillFrame.isNull())
        return MAX_FRAMES;

    const auto frames = lookup(name, LookupMode::Partial);
    for (int frameNumber = 0; frameNumber < frames.size(); frameNumber++) {
        if (stillFrame == frames[frameNumber])
            return frameNumber;
    }
    return MAX_FRAMES;
}

bool ImageManager::VideoThumbnailCache::setStillFrame(const DB::FileName &name, int frameNumber)
{
    if (0 > frameNumber || frameNumber >= MAX_FRAMES)
        return false;

    const auto newImageName = frameName(name, frameNumber);
    if (!newImageName.exists()) {
        qCDebug(ImageManagerLog) << "Video thumbnail still frame (" << newImageName.relative() << ") for" << name.relative() << "could not be set to index" << frameNumber;
        return false;
    }

    Utilities::copyOrOverwrite(newImageName.absolute(), stillFrameName(name).absolute());
    qCDebug(ImageManagerLog) << "Video thumbnail still frame for" << name.relative() << "set to index" << frameNumber;
    return true;
}

QString ImageManager::defaultVideoThumbnailDirectory()
{
    return QString::fromLatin1(".videoThumbnails");
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VideoThumbnailCache.cpp"
