#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H
#include <QMutex>
#include "CacheFileInfo.h"
#include <QMap>
#include <QImage>

namespace ImageManager {

class ThumbnailCache :public QObject
{
    Q_OBJECT

public:
    static ThumbnailCache* instance();
    ThumbnailCache();
    void insert( const QString& name, const QImage& image );
    QPixmap lookup( const QString& name ) const;
    bool contains( const QString& name ) const;
    void load();
    void removeThumbnail( const QString& );

public slots:
    void save() const;
    void flush();

private:
    QString fileNameForIndex( int index ) const;
    QString thumbnailPath( const QString& fileName ) const;

    static ThumbnailCache* m_instance;
    QMap<QString, CacheFileInfo> m_map;
    int m_currentFile;
    int m_currentOffset;
    QTimer* m_timer;
    mutable int m_unsaved;
};

}

#endif /* THUMBNAILCACHE_H */

