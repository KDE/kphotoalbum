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
#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

class QPixmap;
class KFileItem;

#include <qobject.h>
#include "RequestQueue.h"
#include "Manager.h"
#include <QEventLoop>
#include <QPointer>

namespace ImageManager
{
class ImageClient;
class ImageRequest;

class VideoManager :public QObject
{
    Q_OBJECT

public:
    static VideoManager& instance();
    void request( ImageRequest* request );
    void stop( ImageClient*, StopAction action );
    bool hasVideoThumbnailSupport() const;
    void removeFullScaleFrame( const QString& fileName );

protected:
    void load( ImageRequest* request );
    void requestLoadNext();
    void sendResult( QImage image );
    void saveFullScaleFrame( const QImage& image );
    bool requestFullScaleFrame( ImageRequest* request );
    QString pathForRequest( const QString& fileName  );

protected slots:
    void slotGotPreview(const KFileItem&, const QPixmap& pixmap );
    void previewFailed();

    void testGotPreview(const KFileItem&, const QPixmap& pixmap );
    void testPreviewFailed();

private:
    VideoManager();
    ~VideoManager();
    RequestQueue _pending;
    ImageRequest* _currentRequest;
    bool _hasVideoSupport;
    mutable QPointer<QEventLoop> _eventLoop;
};

}

#endif /* VIDEOMANAGER_H */

