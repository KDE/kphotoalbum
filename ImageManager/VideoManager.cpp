/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include "ImageManager/ImageClient.h"
#include <kurl.h>
#include "ImageRequest.h"
#include "Utilities/Util.h"
#include <kio/previewjob.h>
#include <Settings/SettingsData.h>
#include <qimage.h>
#include <QPixmap>
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
    KUrl::List list;
    list.append( request->fileName() );
    KIO::PreviewJob* job=KIO::filePreview(list, request->width() );
    job->setIgnoreMaximumSize( true );

    connect(job, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
            this, SLOT(slotGotPreview(const KFileItem&, const QPixmap&)) );
    connect(job, SIGNAL(failed(const KFileItem&)),
            this, SLOT(previewFailed()) );
}

void ImageManager::VideoManager::slotGotPreview(const KFileItem&, const QPixmap& pixmap )
{
    if ( _pending.isRequestStillValid(_currentRequest) ) {
        _currentRequest->setLoadedOK( true );
        QImage img = pixmap.toImage();
        _currentRequest->client()->pixmapLoaded( _currentRequest->fileName(), pixmap.size(), QSize(-1,-1), 0, img, !pixmap.isNull());
    }

    requestLoadNext();
}

void ImageManager::VideoManager::previewFailed()
{
    if ( _pending.isRequestStillValid(_currentRequest) ) {
        QPixmap pix = KIconLoader::global()->loadIcon( QString::fromLatin1("video"), KIconLoader::Desktop,
                                                       Settings::SettingsData::instance()->thumbSize() );
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

#include "VideoManager.moc"
