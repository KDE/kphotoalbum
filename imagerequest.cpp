#include "imagerequest.h"
ImageRequest::ImageRequest() : _null( true ),  _cache( true ),  _client( 0 )
{
}

ImageRequest::ImageRequest( const QString& fileName, const QSize& size, int angle, ImageClient* client )
    : _null( false ),  _fileName( fileName ),  _width( size.width() ),  _height( size.height() ),
      _cache( false ),  _client( client ),  _angle( angle ), _priority( false ), _loadedOK( false )
{
}

bool ImageRequest::loadedOK() const
{
    return _loadedOK;
}

bool ImageRequest::isNull() const
{
    return _null;
}

QString ImageRequest::fileName() const
{
    // We need a lock here to avoid a race condition in Operator T() of QDeepCopy
    QMutexLocker dummy( &_fileNameLock );
    return _fileName;
}

int ImageRequest::width() const
{
    return _width;
}

int ImageRequest::height() const
{
    return _height;
}

bool ImageRequest::operator<( const ImageRequest& other ) const
{
    if (  fileName() != other.fileName() )
        return fileName() < other.fileName();
    else if ( _width != other._width )
        return _width < other._width;
    else if ( _height < other._height )
        return _height < other._height;
    else
        return _angle < other._angle;
}

bool ImageRequest::operator==( const ImageRequest& other ) const
{
    // Compare all atributes but the pixmap.
    return ( _null == other._null && fileName() == other.fileName() &&
             _width == other._width && _height == other._height &&
             _angle == other._angle );
}


void ImageRequest::setCache( bool b )
{
    _cache = b;
}

bool ImageRequest::cache() const
{
    return _cache;
}

ImageClient* ImageRequest::client() const
{
    return _client;
}

int ImageRequest::angle() const
{
    return _angle;
}

QSize ImageRequest::fullSize() const
{
    return _fullSize;
}

void ImageRequest::setFullSize( const QSize& size )
{
    _fullSize = size;
}

void ImageRequest::setLoadedOK( bool ok )
{
    _loadedOK = ok;
}

bool ImageRequest::priority() const
{
    return _priority;
}

void ImageRequest::setPriority( bool b )
{
    _priority = b;
}

bool ImageRequest::stillNeeded() const
{
    return true;
}
