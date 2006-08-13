#include "VideoManager.h"
#include <kurl.h>
#include "ImageRequest.h"
#include <kio/previewjob.h>
#include <Settings/SettingsData.h>
#include <qimage.h>

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
    if ( _currentRequest == 0 )
        load( request );
    else
        _pending.append( request );
}

void ImageManager::VideoManager::load( ImageRequest* request )
{
    _currentRequest = request;
    KURL::List list;
    list.append( request->fileName() );
    KIO::PreviewJob* job=KIO::filePreview(list, Settings::SettingsData::instance()->thumbSize() );
    job->setIgnoreMaximumSize( true );

    connect(job, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
            this, SLOT(slotGotPreview(const KFileItem*, const QPixmap&)) );
    connect(job, SIGNAL(failed(const KFileItem*)),
            this, SLOT(previewFailed()) );
}

void ImageManager::VideoManager::slotGotPreview(const KFileItem*, const QPixmap& pixmap )
{
    _currentRequest->setLoadedOK( true );
    QImage img = pixmap.convertToImage();
    _currentRequest->client()->pixmapLoaded( _currentRequest->fileName(), pixmap.size(), pixmap.size(), 0, img, !pixmap.isNull() );

    requestLoadNext();
}

void ImageManager::VideoManager::previewFailed()
{
    qDebug("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOPS");
}

void ImageManager::VideoManager::requestLoadNext()
{
    _currentRequest = 0;
    if ( _pending.isEmpty() )
        return;

    request( _pending.front() );
    _pending.pop_front();
}

