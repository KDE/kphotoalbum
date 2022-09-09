// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ThumbnailCache.h"

#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>

#include <QBuffer>
#include <QCache>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QMutexLocker>
#include <QPixmap>
#include <QTemporaryFile>
#include <QTimer>

namespace
{

// We split the thumbnails into chunks to avoid a huge file changing over and over again, with a bad hit for backups
constexpr int MAX_FILE_SIZE = 32 * 1024 * 1024;
constexpr int THUMBNAIL_FILE_VERSION_MIN = 4;
// We map some thumbnail files into memory and manage them in a least-recently-used fashion
constexpr size_t LRU_SIZE = 2;

constexpr int THUMBNAIL_CACHE_SAVE_INTERNAL_MS = (5 * 1000);

constexpr auto INDEXFILE_NAME = "thumbnailindex";
}

namespace ImageManager
{
/**
 * The ThumbnailMapping wraps the memory-mapped data of a QFile.
 * Upon initialization with a file name, the corresponding file is opened
 * and its contents mapped into memory (as a QByteArray).
 *
 * Deleting the ThumbnailMapping unmaps the memory and closes the file.
 */
class ThumbnailMapping
{
public:
    explicit ThumbnailMapping(const QString &filename)
        : file(filename)
        , map(nullptr)
    {
        if (!file.open(QIODevice::ReadOnly))
            qCWarning(ImageManagerLog, "Failed to open thumbnail file");

        uchar *data = file.map(0, file.size());
        if (!data || QFile::NoError != file.error()) {
            qCWarning(ImageManagerLog, "Failed to map thumbnail file");
        } else {
            map = QByteArray::fromRawData(reinterpret_cast<const char *>(data), file.size());
        }
    }
    bool isValid()
    {
        return !map.isEmpty();
    }
    // we need to keep the file around to keep the data mapped:
    QFile file;
    QByteArray map;
};

QString defaultThumbnailDirectory()
{
    return QString::fromLatin1(".thumbnails/");
}
}

ImageManager::ThumbnailCache::ThumbnailCache(const QString &baseDirectory)
    : m_baseDir(baseDirectory)
    , m_currentFile(0)
    , m_currentOffset(0)
    , m_timer(new QTimer)
    , m_needsFullSave(true)
    , m_isDirty(false)
    , m_memcache(new QCache<int, ThumbnailMapping>(LRU_SIZE))
    , m_currentWriter(nullptr)
{
    if (!m_baseDir.exists()) {
        if (!QDir().mkpath(m_baseDir.path())) {
            qCWarning(ImageManagerLog, "Failed to create thumbnail cache directory!");
        }
    }

    // set a default value for version 4 files and new databases:
    m_thumbnailSize = Settings::SettingsData::instance()->thumbnailSize();

    load();
    connect(this, &ImageManager::ThumbnailCache::doSave, this, &ImageManager::ThumbnailCache::saveImpl);
    connect(m_timer, &QTimer::timeout, this, &ImageManager::ThumbnailCache::saveImpl);
    m_timer->setInterval(THUMBNAIL_CACHE_SAVE_INTERNAL_MS);
    m_timer->setSingleShot(true);
    m_timer->start(THUMBNAIL_CACHE_SAVE_INTERNAL_MS);
}

ImageManager::ThumbnailCache::~ThumbnailCache()
{
    m_needsFullSave = true;
    saveInternal();
    delete m_memcache;
    delete m_timer;
    if (m_currentWriter)
        delete m_currentWriter;
}

void ImageManager::ThumbnailCache::insert(const DB::FileName &name, const QImage &image)
{
    if (image.isNull()) {
        qCWarning(ImageManagerLog) << "Thumbnail for file" << name.relative() << "is invalid!";
        return;
    }

    QByteArray data;
    QBuffer buffer(&data);
    bool OK = buffer.open(QIODevice::WriteOnly);
    Q_ASSERT(OK);
    Q_UNUSED(OK);

    OK = image.save(&buffer, "JPG");
    Q_ASSERT(OK);

    insert(name, data);
}

void ImageManager::ThumbnailCache::insert(const DB::FileName &name, const QByteArray &thumbnailData)
{
    if (thumbnailData.isNull()) {
        qCWarning(ImageManagerLog) << "Thumbnail data for file" << name.relative() << "is invalid!";
        return;
    }
    QMutexLocker thumbnailLocker(&m_thumbnailWriterLock);
    if (!m_currentWriter) {
        m_currentWriter = new QFile(fileNameForIndex(m_currentFile));
        if (!m_currentWriter->open(QIODevice::ReadWrite)) {
            qCWarning(ImageManagerLog, "Failed to open thumbnail file for inserting");
            return;
        }
    }
    if (!m_currentWriter->seek(m_currentOffset)) {
        qCWarning(ImageManagerLog, "Failed to seek in thumbnail file");
        return;
    }

    QMutexLocker dataLocker(&m_dataLock);
    // purge in-memory cache for the current file:
    m_memcache->remove(m_currentFile);

    const int sizeBytes = thumbnailData.size();
    if (!(m_currentWriter->write(thumbnailData.data(), sizeBytes) == sizeBytes && m_currentWriter->flush())) {
        qCWarning(ImageManagerLog, "Failed to write image data to thumbnail file");
        return;
    }

    if (m_currentOffset + sizeBytes > MAX_FILE_SIZE) {
        delete m_currentWriter;
        m_currentWriter = nullptr;
    }
    thumbnailLocker.unlock();

    if (m_hash.contains(name)) {
        CacheFileInfo info = m_hash[name];
        if (info.fileIndex == m_currentFile && info.offset == m_currentOffset && info.size == sizeBytes) {
            qCDebug(ImageManagerLog) << "Found duplicate thumbnail " << name.relative() << "but no change in information";
            dataLocker.unlock();
            return;
        } else {
            // File has moved; incremental save does no good.
            // Either the image file has changed and with it the thumbnail, or
            // this is a video file and a different frame has been selected as thumbnail
            qCDebug(ImageManagerLog) << "Setting new thumbnail for image " << name.relative() << ", need full save! ";
            QMutexLocker saveLocker(&m_saveLock);
            m_needsFullSave = true;
        }
    }

    m_hash.insert(name, CacheFileInfo(m_currentFile, m_currentOffset, sizeBytes));
    m_isDirty = true;

    m_unsavedHash.insert(name, CacheFileInfo(m_currentFile, m_currentOffset, sizeBytes));

    // Update offset
    m_currentOffset += sizeBytes;
    if (m_currentOffset > MAX_FILE_SIZE) {
        m_currentFile++;
        m_currentOffset = 0;
    }
    int unsaved = m_unsavedHash.count();
    dataLocker.unlock();

    // Thumbnail building is a lot faster now.  Even on an HDD this corresponds to less
    // than 1 minute of work.
    //
    // We need to call the internal version that does not interact with the timer.
    // We can't simply signal from here because if we're in the middle of loading new
    // images the signal won't get invoked until we return to the main application loop.
    if (unsaved >= 100) {
        saveInternal();
    }
}

QString ImageManager::ThumbnailCache::fileNameForIndex(int index) const
{
    return thumbnailPath(QString::fromLatin1("thumb-") + QString::number(index));
}

QPixmap ImageManager::ThumbnailCache::lookup(const DB::FileName &name) const
{
    auto array = lookupRawData(name);
    if (array.isNull())
        return QPixmap();

    QBuffer buffer(&array);
    buffer.open(QIODevice::ReadOnly);
    QImage image;
    image.load(&buffer, "JPG");

    // Notice the above image is sharing the bits with the file, so I can't just return it as it then will be invalid when the file goes out of scope.
    // PENDING(blackie) Is that still true?
    return QPixmap::fromImage(image);
}

QByteArray ImageManager::ThumbnailCache::lookupRawData(const DB::FileName &name) const
{
    m_dataLock.lock();
    CacheFileInfo info = m_hash[name];
    m_dataLock.unlock();

    ThumbnailMapping *t = m_memcache->object(info.fileIndex);
    if (!t || !t->isValid()) {
        t = new ThumbnailMapping(fileNameForIndex(info.fileIndex));
        if (!t->isValid()) {
            delete t;
            qCWarning(ImageManagerLog, "Failed to map thumbnail file");
            return QByteArray();
        }
        m_memcache->insert(info.fileIndex, t);
    }
    QByteArray array(t->map.mid(info.offset, info.size));
    return array;
}

void ImageManager::ThumbnailCache::saveFull()
{
    QElapsedTimer timer;
    timer.start();
    // First ensure that any dirty thumbnails are written to disk
    QMutexLocker thumbnailLocker(&m_thumbnailWriterLock);
    if (m_currentWriter) {
        delete m_currentWriter;
        m_currentWriter = nullptr;
    }
    thumbnailLocker.unlock();

    QMutexLocker dataLocker(&m_dataLock);
    if (!m_isDirty) {
        qCDebug(ImageManagerLog) << "ThumbnailCache::saveFull(): cache not dirty.";
        return;
    }
    QTemporaryFile file;
    if (!file.open()) {
        qCWarning(ImageManagerLog, "Failed to create temporary file");
        return;
    }
    QHash<DB::FileName, CacheFileInfo> tempHash = m_hash;

    m_unsavedHash.clear();
    m_needsFullSave = false;
    // Clear the dirty flag early so that we can allow further work to proceed.
    // If the save fails, we'll set the dirty flag again.
    m_isDirty = false;
    m_fileVersion = preferredFileVersion();
    dataLocker.unlock();

    QDataStream stream(&file);
    stream << preferredFileVersion()
           << m_thumbnailSize
           << m_currentFile
           << m_currentOffset
           << m_hash.count();

    for (auto it = tempHash.constBegin(); it != tempHash.constEnd(); ++it) {
        const CacheFileInfo &cacheInfo = it.value();
        stream << it.key().relative()
               << cacheInfo.fileIndex
               << cacheInfo.offset
               << cacheInfo.size;
    }
    file.close();

    const QString realFileName = thumbnailPath(INDEXFILE_NAME);
    QFile::remove(realFileName);
    bool success = false;
    if (!file.copy(realFileName)) {
        qCWarning(ImageManagerLog, "Failed to copy the temporary file %s to %s", qPrintable(file.fileName()), qPrintable(realFileName));
    } else {
        QFile realFile(realFileName);
        if (!realFile.open(QIODevice::ReadOnly)) {
            qCWarning(ImageManagerLog, "Could not open the file %s for reading!", qPrintable(realFileName));
        } else {
            if (!realFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther)) {
                qCWarning(ImageManagerLog, "Could not set permissions on file %s!", qPrintable(realFileName));
            } else {
                realFile.close();
                qCDebug(ImageManagerLog) << "ThumbnailCache::saveFull(): cache saved.";
                qCDebug(TimingLog, "Saved thumbnail cache with %d images in %f seconds", size(), timer.elapsed() / 1000.0);
                emit saveComplete();
                success = true;
            }
        }
    }
    if (!success) {
        dataLocker.relock();
        m_isDirty = true;
        m_needsFullSave = true;
    }
}

// Incremental save does *not* clear the dirty flag.  We always want to do a full
// save eventually.
void ImageManager::ThumbnailCache::saveIncremental()
{
    QMutexLocker thumbnailLocker(&m_thumbnailWriterLock);
    if (m_currentWriter) {
        delete m_currentWriter;
        m_currentWriter = nullptr;
    }
    thumbnailLocker.unlock();

    QMutexLocker dataLocker(&m_dataLock);
    if (m_unsavedHash.count() == 0) {
        return;
    }
    QHash<DB::FileName, CacheFileInfo> tempUnsavedHash = m_unsavedHash;
    m_unsavedHash.clear();
    m_isDirty = true;

    const QString realFileName = thumbnailPath(INDEXFILE_NAME);
    QFile file(realFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCWarning(ImageManagerLog, "Failed to open thumbnail cache for appending");
        m_needsFullSave = true;
        return;
    }
    QDataStream stream(&file);
    for (auto it = tempUnsavedHash.constBegin(); it != tempUnsavedHash.constEnd(); ++it) {
        const CacheFileInfo &cacheInfo = it.value();
        stream << it.key().relative()
               << cacheInfo.fileIndex
               << cacheInfo.offset
               << cacheInfo.size;
    }
    file.close();
}

void ImageManager::ThumbnailCache::saveInternal()
{
    QMutexLocker saveLocker(&m_saveLock);
    const QString realFileName = thumbnailPath(INDEXFILE_NAME);
    // If something has asked for a full save, do it!
    if (m_needsFullSave || !QFile(realFileName).exists()) {
        saveFull();
    } else {
        saveIncremental();
    }
}

void ImageManager::ThumbnailCache::saveImpl()
{
    m_timer->stop();
    saveInternal();
    m_timer->setInterval(THUMBNAIL_CACHE_SAVE_INTERNAL_MS);
    m_timer->setSingleShot(true);
    m_timer->start(THUMBNAIL_CACHE_SAVE_INTERNAL_MS);
}

void ImageManager::ThumbnailCache::save()
{
    QMutexLocker saveLocker(&m_saveLock);
    m_needsFullSave = true;
    saveLocker.unlock();
    emit doSave();
}

void ImageManager::ThumbnailCache::load()
{
    QFile file(thumbnailPath(INDEXFILE_NAME));
    if (!file.exists()) {
        qCWarning(ImageManagerLog) << "Thumbnail index file" << file.fileName() << "not found!";
        return;
    }

    QElapsedTimer timer;
    timer.start();
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(ImageManagerLog) << "Could not open thumbnail index file" << file.fileName() << "!";
        return;
    }
    QDataStream stream(&file);
    stream >> m_fileVersion;

    if (m_fileVersion != preferredFileVersion() && m_fileVersion != THUMBNAIL_FILE_VERSION_MIN) {
        qCWarning(ImageManagerLog) << "Thumbnail index version" << m_fileVersion << "can not be used. Discarding...";
        return; // Discard cache
    }

    // We can't allow anything to modify the structure while we're doing this.
    QMutexLocker dataLocker(&m_dataLock);

    if (m_fileVersion == THUMBNAIL_FILE_VERSION_MIN) {
        qCInfo(ImageManagerLog) << "Loading thumbnail index version " << m_fileVersion
                                << "- assuming thumbnail size" << m_thumbnailSize << "px";
    } else {
        stream >> m_thumbnailSize;
        qCDebug(ImageManagerLog) << "Thumbnail cache has thumbnail size" << m_thumbnailSize << "px";
    }

    int expectedCount = 0;
    stream >> m_currentFile
        >> m_currentOffset
        >> expectedCount;
    int count = 0;

    while (!stream.atEnd()) {
        QString name;
        int fileIndex;
        int offset;
        int size;
        stream >> name
            >> fileIndex
            >> offset
            >> size;

        // qCDebug(ImageManagerLog) << "Adding file to index:" << name
        //                          << "(index/offset/size:" << fileIndex << "/" << offset << "/" << size << ")";
        m_hash.insert(DB::FileName::fromRelativePath(name), CacheFileInfo(fileIndex, offset, size));
        if (fileIndex > m_currentFile) {
            m_currentFile = fileIndex;
            m_currentOffset = offset + size;
        } else if (fileIndex == m_currentFile && offset + size > m_currentOffset) {
            m_currentOffset = offset + size;
        }
        if (m_currentOffset > MAX_FILE_SIZE) {
            m_currentFile++;
            m_currentOffset = 0;
        }
        count++;
    }
    qCDebug(TimingLog, "Loaded %d (expected: %d) thumbnails in %f seconds", count, expectedCount, timer.elapsed() / 1000.0);
}

bool ImageManager::ThumbnailCache::contains(const DB::FileName &name) const
{
    QMutexLocker dataLocker(&m_dataLock);
    bool answer = m_hash.contains(name);
    return answer;
}

QString ImageManager::ThumbnailCache::thumbnailPath(const char *utf8FileName) const
{
    return m_baseDir.filePath(QString::fromUtf8(utf8FileName));
}

QString ImageManager::ThumbnailCache::thumbnailPath(const QString &file) const
{
    return m_baseDir.filePath(file);
}

int ImageManager::ThumbnailCache::thumbnailSize() const
{
    return m_thumbnailSize;
}

int ImageManager::ThumbnailCache::actualFileVersion() const
{
    return m_fileVersion;
}

int ImageManager::ThumbnailCache::preferredFileVersion()
{
    return 5;
}

DB::FileNameList ImageManager::ThumbnailCache::findIncorrectlySizedThumbnails() const
{
    QMutexLocker dataLocker(&m_dataLock);
    const QHash<DB::FileName, CacheFileInfo> tempHash = m_hash;
    dataLocker.unlock();

    // accessing the data directly instead of using the lookupRawData() method
    // may be more efficient, but this method should be called rarely
    // and readability therefore trumps performance
    DB::FileNameList resultList;
    for (auto it = tempHash.constBegin(); it != tempHash.constEnd(); ++it) {
        const auto filename = it.key();
        auto jpegData = lookupRawData(filename);
        Q_ASSERT(!jpegData.isNull());

        QBuffer buffer(&jpegData);
        buffer.open(QIODevice::ReadOnly);
        QImage image;
        image.load(&buffer, "JPG");
        const auto size = image.size();
        if (size.width() != m_thumbnailSize && size.height() != m_thumbnailSize) {
            qCDebug(ImageManagerLog) << "Thumbnail for file " << filename.relative() << "has incorrect size:" << size;
            resultList.append(filename);
        }
    }

    return resultList;
}

int ImageManager::ThumbnailCache::size() const
{
    QMutexLocker dataLocker(&m_dataLock);
    return m_hash.size();
}

void ImageManager::ThumbnailCache::vacuum()
{
    QMutexLocker dataLocker(&m_dataLock);
    while (m_isDirty) {
        dataLocker.unlock();
        saveFull();
        dataLocker.relock();
    }
    QElapsedTimer timer;
    timer.start();

    long oldStorageSize = 0;
    const auto backupSuffix = QChar::fromLatin1('~');
    // save what we need
    for (int i = 0; i <= m_currentFile; ++i) {
        const auto cacheFile = fileNameForIndex(i);
        oldStorageSize += QFileInfo(cacheFile).size();
        QFile::rename(cacheFile, cacheFile + backupSuffix);
    }

    const int maxFileIndex = m_currentFile;
    // we need to store the filename besides the cache file info so that we can reinsert it later
    struct RichCacheFileInfo {
        CacheFileInfo info;
        DB::FileName name;
    };
    QList<RichCacheFileInfo> cacheEntries;
    for (auto it = m_hash.constKeyValueBegin(); it != m_hash.constKeyValueEnd(); ++it) {
        cacheEntries.append(RichCacheFileInfo { (*it).second, (*it).first });
    }
    // sort for sequential I/O:
    std::sort(cacheEntries.begin(), cacheEntries.end(), [](RichCacheFileInfo a, RichCacheFileInfo b) { return a.info.fileIndex < b.info.fileIndex || (a.info.fileIndex == b.info.fileIndex && a.info.offset < b.info.offset); });

    // flush the cache manually (cache files have been moved already)
    m_currentFile = 0;
    m_currentOffset = 0;
    m_isDirty = true;
    m_hash.clear();
    m_unsavedHash.clear();
    m_memcache->clear();
    dataLocker.unlock();

    // rebuild
    int currentFileIndex { -1 };
    ThumbnailMapping *currentFile { nullptr };
    for (const auto &entry : qAsConst(cacheEntries)) {
        Q_ASSERT(entry.info.fileIndex != -1);
        if (entry.info.fileIndex != currentFileIndex) {
            currentFileIndex = entry.info.fileIndex;
            if (currentFile)
                delete currentFile;
            currentFile = new ThumbnailMapping(fileNameForIndex(currentFileIndex) + backupSuffix);
        }

        const QByteArray imageData(currentFile->map.mid(entry.info.offset, entry.info.size));
        insert(entry.name, imageData);
    }
    if (currentFile)
        delete currentFile;

    qCDebug(TimingLog, "Rewrote %d thumbnails in %f seconds", size(), timer.elapsed() / 1000.0);
    long newStorageSize = 0;
    for (int i = 0; i <= m_currentFile; ++i) {
        const auto cacheFile = fileNameForIndex(i);
        newStorageSize += QFileInfo(cacheFile).size();
    }
    qCDebug(ImageManagerLog, "Thumbnail storage used %ld bytes in %d files before and %ld bytes in %d files after operation.", oldStorageSize, maxFileIndex, newStorageSize, m_currentFile);
    qCDebug(ImageManagerLog, "Size reduction: %.2f%%", 100.0 * (oldStorageSize - newStorageSize) / oldStorageSize);
    for (int i = 0; i <= maxFileIndex; ++i) {
        const auto cacheFile = fileNameForIndex(i);
        QFile::remove(cacheFile + backupSuffix);
    }
    save();
}

void ImageManager::ThumbnailCache::flush()
{
    QMutexLocker dataLocker(&m_dataLock);
    for (int i = 0; i <= m_currentFile; ++i)
        QFile::remove(fileNameForIndex(i));
    m_currentFile = 0;
    m_currentOffset = 0;
    m_isDirty = true;
    m_hash.clear();
    m_unsavedHash.clear();
    m_memcache->clear();
    dataLocker.unlock();
    save();
    emit cacheFlushed();
}

void ImageManager::ThumbnailCache::removeThumbnail(const DB::FileName &fileName)
{
    QMutexLocker dataLocker(&m_dataLock);
    m_isDirty = true;
    m_hash.remove(fileName);
    dataLocker.unlock();
    save();
}
void ImageManager::ThumbnailCache::removeThumbnails(const DB::FileNameList &files)
{
    QMutexLocker dataLocker(&m_dataLock);
    m_isDirty = true;
    for (const DB::FileName &fileName : files) {
        m_hash.remove(fileName);
    }
    dataLocker.unlock();
    save();
}

void ImageManager::ThumbnailCache::setThumbnailSize(int thumbSize)
{
    if (thumbSize < 0)
        return;

    if (thumbSize != m_thumbnailSize) {
        m_thumbnailSize = thumbSize;
        flush();
        emit cacheInvalidated();
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ThumbnailCache.cpp"
