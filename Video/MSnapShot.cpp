#include "MSnapShot.h"
#include "ImageManager/ImageRequest.h"
#include "ImageManager/ImageLoader.h"
#include <qdir.h>
#include <qstringlist.h>
#include <qimage.h>
#include <kglobal.h>
#include <kiconloader.h>

Video::MSnapShot::MSnapShot()
{
    connect( &_process, SIGNAL(processExited(KProcess *)), this, SLOT( processDone() ) );
}

void Video::MSnapShot::load( ImageManager::ImageRequest* request )
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

QString Video::MSnapShot::tmpDir() const
{
    return QString::fromLatin1( "/tmp/frameout" ); // PENDING(blackie) VIDEO do not hardcode
}

void Video::MSnapShot::processDone()
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

void Video::MSnapShot::startNextLoad()
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
    qDebug( "%s", QString::fromLatin1("%1 %2 %3 %4 %5 %6 %7").arg(QString::fromLatin1( "mplayer" )).arg(QString::fromLatin1( "-vo" ))
            .arg( QString::fromLatin1( "jpeg:outdir=%1" ).arg( tmpDir() ) )
            .arg( QString::fromLatin1( "-frames" ) ).arg( QString::fromLatin1( "1" ) )
            .arg( _current->fileName() ).latin1());

    _process.start();
}

bool Video::MSnapShot::tryLoadThumbnail( ImageManager::ImageRequest* request )
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

void Video::MSnapShot::sendAnswer( QImage image, ImageManager::ImageRequest* request )
{
    QSize origSize = image.size();

    image = ImageManager::ImageLoader::rotateAndScale( image, request->width(), request->height(), request->angle() );

    request->client()->pixmapLoaded( request->fileName(), QSize( request->width(), request->height() ), origSize,
                                      request->angle(), image, !image.isNull() );

}

