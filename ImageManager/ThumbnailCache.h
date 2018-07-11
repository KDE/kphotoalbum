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
#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H
#include "CacheFileInfo.h"
#include <QHash>
#include <QImage>
#include <DB/FileNameList.h>
#include <QMutex>
#include <QFile>

template <class Key, class T>
class QCache;

namespace ImageManager {

class ThumbnailMapping;

class ThumbnailCache :public QObject
{
    Q_OBJECT

public:
    static ThumbnailCache* instance();
    static void deleteInstance();
    ThumbnailCache();
    void insert( const DB::FileName& name, const QImage& image );
    QPixmap lookup( const DB::FileName& name ) const;
    QByteArray lookupRawData( const DB::FileName& name ) const;
    bool contains( const DB::FileName& name ) const;
    void load();
    void removeThumbnail( const DB::FileName& );
    void removeThumbnails( const DB::FileNameList& );

public slots:
    void save() const;
    void flush();

signals:
    void doSave() const;

private:
    ~ThumbnailCache();
    QString fileNameForIndex( int index, const QString dir = QString::fromLatin1(".thumbnails/") ) const;
    QString thumbnailPath( const QString& fileName, const QString dir = QString::fromLatin1(".thumbnails/") ) const;

    static ThumbnailCache* s_instance;
    QHash<DB::FileName, CacheFileInfo> m_hash;
    mutable QHash<DB::FileName, CacheFileInfo> m_unsavedHash;
    /* Protects accesses to the data (hash and unsaved hash) */
    mutable QMutex m_dataLock;
    /* Prevents multiple saves from happening simultaneously */
    mutable QMutex m_saveLock;
    /* Protects writing thumbnails to disk */
    mutable QMutex m_thumbnailWriterLock;
    int m_currentFile;
    int m_currentOffset;
    mutable QTimer* m_timer;
    mutable bool m_needsFullSave;
    mutable bool m_isDirty;
    void saveFull() const;
    void saveIncremental() const;
    void saveInternal() const;
    void saveImpl() const;

    /**
     * Holds an in-memory cache of thumbnail files.
     */
    mutable QCache<int,ThumbnailMapping> *m_memcache;
    mutable QFile* m_currentWriter;
};

}

#endif /* THUMBNAILCACHE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
