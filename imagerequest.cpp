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
    return const_cast<ImageRequest*>(this)->_fileName;
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
    ImageRequest& o = const_cast<ImageRequest&>( other );
    ImageRequest& t = const_cast<ImageRequest&>( *this );

    if ( (QString&) t._fileName != (QString&)o._fileName )
        return t._fileName < o._fileName;
    else if ( t._width != o._width )
        return t._width < o._width;
    else if ( t._height < o._height )
        return t._height < o._height;
    else
        return t._angle < o._angle;
}

bool ImageRequest::operator==( const ImageRequest& other ) const
{
    // Compare all atributes but the pixmap.
    ImageRequest& t = const_cast<ImageRequest&>( *this );
    ImageRequest& o = const_cast<ImageRequest&>( other );
    return ( t._null == o._null && t._fileName == o._fileName &&
             t._width == o._width && t._height == o._height &&
             t._angle == o._angle );
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

