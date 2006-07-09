#include "VideoLoader.h"
#include "ImageManager/ImageRequest.h"
#include "ImageManager/ImageLoader.h"
#include <qdir.h>
#include <qstringlist.h>
#include <qimage.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qpainter.h>
#include <kstandarddirs.h>

ImageManager::VideoLoader::VideoLoader( ImageRequest* request ) : _current( 0 )
{
    load( request );
    connect( &_process, SIGNAL(processExited(KProcess *)), this, SLOT( processDone() ) );
}


void ImageManager::VideoLoader::load( ImageManager::ImageRequest* request )
{
    if ( _current && *_current == *request )
        return; // We are currently loading it, calm down and wait please ;-)

    // Check pending list
    for( QValueList<ImageManager::ImageRequest*>::Iterator it = _pendingRequest.begin(); it != _pendingRequest.end(); ) {
        if ( *(*it) == *request )
            return; // This image is already in the queue
        else
            ++it;
    }

    if ( tryLoadThumbnail( request ) )
        return;

    _pendingRequest.append( request );
    if ( _current == 0 )
        startNextLoad();
}

QString ImageManager::VideoLoader::tmpDir() const
{
    return QString::fromLatin1( "/tmp/frameout" ); // PENDING(blackie) VIDEO do not hardcode
}

void ImageManager::VideoLoader::processDone()
{
    QDir dir( tmpDir() );

    QStringList files = dir.entryList( QString::fromLatin1( "*.jpg" ) );
    QImage result;

    if ( files.count() == 0 )
        result = KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "video" ), KIcon::Desktop ).convertToImage();

    else {
        result = QImage( QString::fromLatin1( "%1/%2" ).arg(tmpDir()).arg(files[0]) );

        for( QStringList::Iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
            dir.remove( QString::fromLatin1( "%1/%2" ).arg(tmpDir()).arg(*fileIt ), true );
        }

        dir.rmdir( tmpDir(), true );
    }

    ImageManager::ImageLoader::writeThumbnail( _current, result );
    sendAnswer( result, _current );
    delete _current;
    _current = 0;

    startNextLoad();
}

void ImageManager::VideoLoader::startNextLoad()
{
    if ( _pendingRequest.count() == 0 )
        return;

    _current = _pendingRequest.front();
    _pendingRequest.pop_front();

    _process.clearArguments();
    _process << QString::fromLatin1( "mplayer" ) << QString::fromLatin1( "-vo" )
             << QString::fromLatin1( "jpeg:outdir=%1" ).arg( tmpDir() )
             << QString::fromLatin1( "-frames" ) << QString::fromLatin1( "1" )
             << QString::fromLatin1( "-ao" ) << QString::fromLatin1( "null" )
             << _current->fileName();

    _process.start();
}

bool ImageManager::VideoLoader::tryLoadThumbnail( ImageManager::ImageRequest* request )
{
    bool ok;
    QImage image = ImageManager::ImageLoader::tryLoadThumbnail( request, ok );
    if ( ok ) {
        sendAnswer( image, request );
        delete request;
        return true;
    }
    else
        return false;
}

void ImageManager::VideoLoader::sendAnswer( QImage image, ImageManager::ImageRequest* request )
{
    QSize origSize = image.size();

    image = ImageManager::ImageLoader::rotateAndScale( image, request->width(), request->height(), request->angle() );

    image = drawMovieClip( image );
    request->client()->pixmapLoaded( request->fileName(), QSize( request->width(), request->height() ), origSize,
                                      request->angle(), image, !image.isNull() );

}

QImage ImageManager::VideoLoader::drawMovieClip( const QImage & image)
{
    QPixmap pix( image );

    QPainter painter( &pix );
    QPixmap clip( locate("data", QString::fromLatin1("kphotoalbum/pics/movie-clip.png") ) );

    painter.drawPixmap( image.width() - clip.width(), image.height() - clip.height(), clip );

    return pix.convertToImage();
}
