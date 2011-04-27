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
#include "VideoManager.h"
#include "ImageManager/ThumbnailCache.h"
#include "ImageManager/ImageClient.h"
#include <kurl.h>
#include "ImageRequest.h"
#include "Utilities/Util.h"
#include <kio/previewjob.h>
#include <Settings/SettingsData.h>
#include <qimage.h>
#include <QPixmap>
#include <kiconloader.h>
#include "Settings/SettingsData.h"
#include <kcodecs.h>
#include <QDir>

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
    const QImage image = loadFullScaleFrame(request);
    if ( !image.isNull()) {
        sendResult(image);
        return;
    }
    _currentRequest = request;
    KUrl::List list;
    list.append( request->fileName() );
    // All the extra parameters are the defaults. I need the last false,
    // which says "Don't cache". If it caches, then I wont get a new shot
    // when the user chooses load new thumbnail
    KIO::PreviewJob* job=KIO::filePreview(list, 1024, 0,0, 100, true, false );

    job->setIgnoreMaximumSize( true );

    connect(job, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
            this, SLOT(slotGotPreview(const KFileItem&, const QPixmap&)) );
    connect(job, SIGNAL(failed(const KFileItem&)),
            this, SLOT(previewFailed()) );
}

void ImageManager::VideoManager::slotGotPreview(const KFileItem&, const QPixmap& pixmap )
{
    const QImage image = pixmap.toImage();
    saveFullScaleFrame(image);
    sendResult(image);
}

void ImageManager::VideoManager::previewFailed()
{
    if ( _pending.isRequestStillValid(_currentRequest) ) {
        QPixmap pix = KIconLoader::global()->loadIcon( QString::fromLatin1("video"), KIconLoader::Desktop,
                                                       Settings::SettingsData::instance()->thumbSize() );
        if ( _currentRequest->isThumbnailRequest() )
            ImageManager::ThumbnailCache::instance()->insert( _currentRequest->fileName(), pix.toImage() );

        _currentRequest->setLoadedOK( false );
        _currentRequest->client()->pixmapLoaded( _currentRequest->fileName(), pix.size(), pix.size(), 0, pix.toImage(), true);
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


bool ImageManager::VideoManager::hasVideoThumbnailSupport() const
{
    KUrl::List list;
    list.append(Utilities::locateDataFile(QString::fromLatin1("demo/movie.avi")));
    KIO::PreviewJob* job=KIO::filePreview(list, 64 );
    job->setIgnoreMaximumSize( true );

    connect(job, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
            this, SLOT(testGotPreview(const KFileItem&, const QPixmap&)) );
    connect(job, SIGNAL(failed(const KFileItem&)),
            this, SLOT(testPreviewFailed()) );

    _eventLoop.exec();
    return _hasVideoSupport;
}

void ImageManager::VideoManager::testGotPreview(const KFileItem&, const QPixmap& pixmap )
{
    _hasVideoSupport = !pixmap.isNull();
    _eventLoop.exit();
}

void ImageManager::VideoManager::testPreviewFailed()
{
    _hasVideoSupport = false;
    _eventLoop.exit();
}

void ImageManager::VideoManager::sendResult(QImage image)
{
    if ( _pending.isRequestStillValid(_currentRequest) ) {
        image = image.scaled( QSize(_currentRequest->width(), _currentRequest->height()), Qt::KeepAspectRatio, Qt::SmoothTransformation );
        if ( _currentRequest->isThumbnailRequest() )
            ImageManager::ThumbnailCache::instance()->insert( _currentRequest->fileName(), image );
        _currentRequest->setLoadedOK( true );
        _currentRequest->client()->pixmapLoaded( _currentRequest->fileName(), image.size(), QSize(-1,-1), 0, image, !image.isNull());
    }

    requestLoadNext();
}

void ImageManager::VideoManager::saveFullScaleFrame(const QImage &image)
{
    QDir dir( Settings::SettingsData::instance()->imageDirectory() );
    if ( !dir.exists(QString::fromLatin1(".videoThumbnails")))
        dir.mkdir(QString::fromLatin1(".videoThumbnails"));
    image.save(pathForRequest(_currentRequest->fileName()), "JPEG");
}

QImage ImageManager::VideoManager::loadFullScaleFrame(ImageManager::ImageRequest *request)
{
    const QString path = pathForRequest(request->fileName());
    if ( QFile::exists(path) ) {
        QImage img;
        img.load(path);
        return img;
    }
    return QImage();
}

QString ImageManager::VideoManager::pathForRequest(const QString& fileName )
{
    KMD5 md5(fileName.toUtf8());
    return QString::fromLatin1("%1/.videoThumbnails/%2").arg(Settings::SettingsData::instance()->imageDirectory()).arg(QString::fromUtf8(md5.hexDigest()));
}

void ImageManager::VideoManager::removeFullScaleFrame(const QString &fileName)
{
    QDir().remove(pathForRequest(fileName));
}

#include "VideoManager.moc"
