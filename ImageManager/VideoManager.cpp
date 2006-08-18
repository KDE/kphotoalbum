#include "VideoManager.h"
#include <kurl.h>
#include "ImageRequest.h"
#include <kio/previewjob.h>
#include <Settings/SettingsData.h>
#include <qimage.h>
#include <kglobal.h>
#include <kiconloader.h>

ImageManager::VideoManager::VideoManager()
    :_currentRequest(0)
{
}

ImageManager::VideoManager& ImageManager::VideoManager::instance()
{
    static VideoManager manager;
    return manager;
}

void ImageManager::VideoManager::request( ImageRequest* request )
{
    _pending.addRequest( request );

    if ( _currentRequest == 0 )
        requestLoadNext();
}

void ImageManager::VideoManager::load( ImageRequest* request )
{
    _currentRequest = request;
    KURL::List list;
    list.append( request->fileName() );
    KIO::PreviewJob* job=KIO::filePreview(list, request->width() );
    job->setIgnoreMaximumSize( true );

    connect(job, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
            this, SLOT(slotGotPreview(const KFileItem*, const QPixmap&)) );
    connect(job, SIGNAL(failed(const KFileItem*)),
            this, SLOT(previewFailed()) );
}

void ImageManager::VideoManager::slotGotPreview(const KFileItem*, const QPixmap& pixmap )
{
    if ( _pending.isRequestStillValid(_currentRequest) ) {
        _currentRequest->setLoadedOK( true );
        QImage img = pixmap.convertToImage();
        _currentRequest->client()->pixmapLoaded( _currentRequest->fileName(), pixmap.size(), pixmap.size(), 0, img, !pixmap.isNull() );
    }

    requestLoadNext();
}

void ImageManager::VideoManager::previewFailed()
{
    if ( _pending.isRequestStillValid(_currentRequest) ) {
        QPixmap pix = KGlobal::iconLoader()->loadIcon( QString::fromLatin1("video"), KIcon::Desktop,
                                                       Settings::SettingsData::instance()->thumbSize() );
        _currentRequest->setLoadedOK( false );
        _currentRequest->client()->pixmapLoaded( _currentRequest->fileName(), pix.size(), pix.size(), 0, pix.convertToImage(), true );
    }

    requestLoadNext();
}

void ImageManager::VideoManager::requestLoadNext()
{
    if ( _currentRequest )
        _pending.removeRequest( _currentRequest );
    _currentRequest = _pending.popNext();
    if ( _currentRequest )
        load( _currentRequest );
}

void ImageManager::VideoManager::stop( ImageClient* client, StopAction action )
{
    _pending.cancelRequests( client, action );
}


#include "VideoManager.moc"
