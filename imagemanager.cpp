#include "imagemanager.h"
#include "imageloader.h"
#include "options.h"
#include "imageclient.h"
#include <qdatetime.h>
#include <qmutex.h>

ImageManager* ImageManager::_instance = 0;

ImageManager::ImageManager()
{
}

// We need this as a separate method as the _instance variable will otherwise not be initialized
// corrected before the thread starts.
void ImageManager::init()
{
    _sleepers = new QWaitCondition(); // Is it necessary to load this using new?
    _lock = new QMutex(); // necessary with new?

    // Only use 1 image loader thread as the JPEG loader is not thread safe
    _imageLoader = new ImageLoader( _sleepers );
    _imageLoader->start();
}

void ImageManager::load( const QString& fileName, ImageClient* client, int angle, int width, int height,
                         bool cache, bool priority, bool compress )
{
    QString key = QString::fromLatin1("%1-%2x%3-%4").arg( fileName ).arg( width ).arg( height ).arg( angle );

    _lock->lock();
    LoadInfo li( fileName, width, height, angle, compress, client );
    li.setCache( cache );
    if ( priority )
        _loadList.prepend( li );
    else
        _loadList.append( li );
    _lock->unlock();
    if ( client )
        _clientMap.insert( li, client );
    _sleepers->wakeOne();
}

LoadInfo ImageManager::next()
{
    _lock->lock();
    LoadInfo info;
    if ( _loadList.count() != 0 )  {
        info = _loadList.first();
        _loadList.pop_front();
    }
    _lock->unlock();
    return info;
}

LoadInfo::LoadInfo() : _null( true ),  _cache( true ),  _client( 0 )
{
}

LoadInfo::LoadInfo( const QString& fileName, int width, int height, int angle, bool compress,
                    ImageClient* client )
    : _null( false ),  _fileName( fileName ),  _width( width ),  _height( height ),
      _cache( true ),  _client( client ),  _angle( angle ),
      _compress( compress )
{
}

void ImageManager::customEvent( QCustomEvent* ev )
{
    if ( ev->type() == 1001 )  {
        ImageEvent* iev = dynamic_cast<ImageEvent*>( ev );
        if ( !iev )  {
            Q_ASSERT( iev );
            return;
        }

        LoadInfo li = iev->loadInfo();
        QString key = QString::fromLatin1("%1-%2x%3-%4").arg( li.fileName() )
                      .arg( li.width() ).arg( li.height() ).arg( li.angle() );

        QImage image = iev->image();
        if ( _clientMap.contains( li ) )  {
            // If it is not in the map, then it has been deleted since the request.
            ImageClient* client = _clientMap[li];

            client->pixmapLoaded( li.fileName(), li.width(), li.height(), li.angle(), image );
            _clientMap.remove(li);
        }
    }
}

ImageEvent::ImageEvent( LoadInfo info, const QImage& image )
    : QCustomEvent( 1001 ), _info( info ),  _image( image )
{
}

LoadInfo ImageEvent::loadInfo()
{
    return _info;
}

bool LoadInfo::isNull() const
{
    return _null;
}

QString LoadInfo::fileName() const
{
    return const_cast<LoadInfo*>(this)->_fileName;
}

int LoadInfo::width() const
{
    return _width;
}

int LoadInfo::height() const
{
    return _height;
}

ImageManager* ImageManager::instance()
{
    if ( !_instance )  {
        _instance = new ImageManager;
        _instance->init();
    }

    return _instance;
}

bool LoadInfo::operator<( const LoadInfo& other ) const
{
    LoadInfo& o = const_cast<LoadInfo&>( other );
    LoadInfo& t = const_cast<LoadInfo&>( *this );

    if ( (QString&) t._fileName != (QString&)o._fileName )
        return t._fileName < o._fileName;
    else if ( t._width != o._width )
        return t._width < o._width;
    else if ( t._height < o._height )
        return t._height < o._height;
    else
        return t._angle < o._angle;
}

bool LoadInfo::operator==( const LoadInfo& other ) const
{
    // Compare all atributes but the pixmap.
    LoadInfo& t = const_cast<LoadInfo&>( *this );
    LoadInfo& o = const_cast<LoadInfo&>( other );
    return ( t._null == o._null && t._fileName == o._fileName &&
             t._width == o._width && t._height == o._height &&
             t._angle == o._angle );
}


void LoadInfo::setCache( bool b )
{
    _cache = b;
}

bool LoadInfo::cache() const
{
    return _cache;
}

void ImageManager::stop( ImageClient* client )
{
    // remove from active map
    for( QMapIterator<LoadInfo,ImageClient*> it= _clientMap.begin(); it != _clientMap.end(); ) {
        LoadInfo key = it.key();
        ImageClient* data = it.data();
        ++it; // We need to increase it before removing the element.
        if ( data == client )
            _clientMap.remove( key );
    }

    // remove from pending map.
    _lock->lock();
    for( QValueList<LoadInfo>::Iterator it = _loadList.begin(); it != _loadList.end(); ) {
        LoadInfo li = *it;
        ++it;
        if ( li.client() == client )
            _loadList.remove( li );
    }
    _lock->unlock();
}

ImageClient* LoadInfo::client()
{
    return _client;
}

QImage ImageEvent::image()
{
    return _image;
}

int LoadInfo::angle() const
{
    return _angle;
}



bool LoadInfo::compress() const
{
    return _compress;
}

#include "imagemanager.moc"
