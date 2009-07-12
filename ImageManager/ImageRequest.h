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
#ifndef IMAGEREQUEST_H
#define IMAGEREQUEST_H
#include <qstring.h>
#include <q3deepcopy.h>
#include <qsize.h>
#include <qmutex.h>
#include <QHash>

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This class is shared among the image loader thead and the GUI tread, if
// you don't know the implication of this stay out of this class!
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

namespace ImageManager
{
class ImageClient;

/** @short Priority of an image request
 *
 * The higher the priority, the sooner the image is expected to be decoded
 * */
enum Priority {
    BuildThumbnails, //< @short Requests generated through the "Rebuild Thumbnails" command
    BuildScopeThumbnails, //< @short thumbnails in current search scope to be rebuidl
    ThumbnailInvisible, //< @short Thumbnails in current search scope, but invisible
    ViewerPreload, // < @short Image that will be displayed later
    BatchTask, /**< @short Requests like resizing images for HTML pages
                *
                * As they are requested by user, they are expected to finish
                * sooner than invisible thumbnails */
    ThumbnailVisible, /**< @short Thumbnail visible on screen right now (might get invalidated later) */
    Viewer /**< @short Image is visible in the viewer right now */,
    LastPriority /**< @short Boundary for list of queues */
};

class ImageRequest {
public:
    ImageRequest( const QString& fileName, const QSize& size, int angle, ImageClient* client);
    virtual ~ImageRequest() {}

    bool isNull() const;
    QString fileName() const;
    int width() const;
    int height() const;
    int angle() const;

    ImageClient* client() const;

    QSize fullSize() const;
    void setFullSize( const QSize& );
    void setLoadedOK( bool ok );
    bool loadedOK() const;

    void setPriority( const Priority prio );
    Priority priority() const;

    bool operator<( const ImageRequest& other ) const;
    bool operator==( const ImageRequest& other ) const;

    virtual bool stillNeeded() const;

    bool doUpScale() const;
    void setUpScale( bool b );

private:
    bool _null;
    mutable Q3DeepCopy<QString> _fileName; // PENDING(blackie) I don't think this deep copy is needed anymore!
    mutable QMutex _fileNameLock;

    int _width;
    int _height;
    ImageClient* _client;
    int _angle;
    QSize _fullSize;
    Priority _priority;
    bool _loadedOK;
    bool _dontUpScale;
};

inline uint qHash(const ImageRequest& ir)
{
    return ::qHash(ir.fileName()) ^ ::qHash(ir.width()) ^ ::qHash(ir.angle());
}

}

#endif /* IMAGEREQUEST_H */
