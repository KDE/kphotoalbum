#include "ThumbnailCache.h"
#include <QTemporaryFile>
#include <QMutexLocker>
#include <QDir>
#include <Settings/SettingsData.h>
#include <QTimer>
#include <QPixmap>
#include <QFile>

const int MAXFILESIZE=32*1024*1024;
const int FILEVERSION=1;

ImageManager::ThumbnailCache* ImageManager::ThumbnailCache::m_instance = 0;

ImageManager::ThumbnailCache::ThumbnailCache()
    : m_currentFile(0), m_currentOffset(0), m_unsaved(0)
{
    const QString dir = thumbnailPath(QString());
    if ( !QFile::exists(dir) )
        QDir().mkpath(dir);

    load();
    m_timer = new QTimer;
    connect( m_timer, SIGNAL(timeout()), this, SLOT(save()));
}

void ImageManager::ThumbnailCache::insert( const QString& name, const QImage& image )
{
    QFile file( fileNameForIndex(m_currentFile) );
    file.open(QIODevice::ReadWrite );
    file.seek( m_currentOffset );

    // PENDING(blackie) all my images should simply be in this format.
    QImage image2 = image.convertToFormat( QImage::Format_RGB32 );
    const int size = image2.byteCount();

    file.write( (char*) image2.bits(), size );
    file.close();

    m_map.insert( name, CacheFileInfo( m_currentFile, m_currentOffset, image.width(), image.height() ) );

    // Update offset
    m_currentOffset += size;
    if ( m_currentOffset > MAXFILESIZE ) {
        m_currentFile++;
        m_currentOffset = 0;
    }

    if ( ++m_unsaved > 100 )
        save();
    m_timer->start(1000);
}

QString ImageManager::ThumbnailCache::fileNameForIndex( int index ) const
{
    return thumbnailPath(QString::fromLatin1("thumb-") + QString::number(index) );
}

QPixmap ImageManager::ThumbnailCache::lookup( const QString& name ) const
{
    CacheFileInfo info = m_map[name];

    QFile file( fileNameForIndex( info.fileIndex ) );
    file.open( QIODevice::ReadOnly );

    uchar* data = file.map( 0, file.size() );
    QImage image( &data[info.offset], info.width, info.height, QImage::Format_RGB32 );

    // Notice the above image is sharing the bits with the file, so I can't just return it as it then will be invalid when the file goes out of scope.
    return QPixmap::fromImage( image );
}

void ImageManager::ThumbnailCache::save() const
{
    m_timer->stop();
    m_unsaved = 0;

    QTemporaryFile file;
    if ( !file.open() ) {
        qWarning("Failed to crate temporary file");
        return;
    }

    QDataStream stream(&file);
    stream << FILEVERSION
           << m_currentFile
           << m_currentOffset
           << m_map.count();

    for( QMap<QString,CacheFileInfo>::ConstIterator it = m_map.begin(); it != m_map.end(); ++it ) {
        const CacheFileInfo& cacheInfo = it.value();
        stream << it.key()
               << cacheInfo.fileIndex
               << cacheInfo.offset
               << cacheInfo.width
               << cacheInfo.height;
    }
    file.close();

    const QString realFileName = thumbnailPath(QString::fromLatin1("thumbnailindex"));
    QFile::remove( realFileName );
    if ( !file.copy( realFileName ) )
        qWarning("Failed to copy the temporary file %s to %s", qPrintable( file.fileName() ), qPrintable( realFileName ) );

}

void ImageManager::ThumbnailCache::load()
{
    QFile file( thumbnailPath( QString::fromLatin1("thumbnailindex")) );
    if ( !file.exists() )
        return;

    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    int version;
    stream >> version;
    if ( version != FILEVERSION )
        qFatal("Ohhh my wrong file version. How did that happen?");

    int count;
    stream >> m_currentFile
           >> m_currentOffset
           >> count;

    for ( int i = 0; i < count; ++i ) {
        QString name;
        int fileIndex;
        int offset;
        int width;
        int height;
        stream >> name
               >> fileIndex
               >> offset
               >> width
               >> height;
        m_map.insert( name, CacheFileInfo( fileIndex, offset, width, height ) );
    }
}

bool ImageManager::ThumbnailCache::contains( const QString& name ) const
{
    return m_map.contains(name);
}

QString ImageManager::ThumbnailCache::thumbnailPath(const QString& file) const
{
    // Making it static is just an optimization.
    static QString base = QDir::home().absoluteFilePath( QString::fromLatin1(".thumbnails/kphotoalbum/") );
    return  base + file;
}

ImageManager::ThumbnailCache* ImageManager::ThumbnailCache::instance()
{
    if (!m_instance)
        m_instance = new ThumbnailCache;
    return m_instance;
}

void ImageManager::ThumbnailCache::flush()
{
    for ( int i = 0; i <= m_currentFile; ++i )
        QFile::remove( fileNameForIndex(i) );
    m_currentFile = 0;
    m_currentOffset = 0;
    m_map.clear();
    save();
}

void ImageManager::ThumbnailCache::removeThumbnail( const QString& fileName )
{
    m_map.remove( fileName );
    save();
}
