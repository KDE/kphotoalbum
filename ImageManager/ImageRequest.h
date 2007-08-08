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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef IMAGEREQUEST_H
#define IMAGEREQUEST_H
#include <qstring.h>
#include "ImageManager/ImageClient.h"
#include <qdeepcopy.h>
#include <qsize.h>
#include <qmutex.h>

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This class is shared among the image loader thead and the GUI tread, if
// you don't know the implication of this stay out of this class!
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

namespace ImageManager
{

class ImageRequest {
public:
    virtual ~ImageRequest() {}
    ImageRequest( const QString& fileName, const QSize& size, int angle, ImageClient* client);

    bool isNull() const;
    QString fileName() const;
    int width() const;
    int height() const;
    int angle() const;

    void setCache( bool b = true );
    bool cache() const;
    ImageClient* client() const;

    QSize fullSize() const;
    void setFullSize( const QSize& );
    void setLoadedOK( bool ok );
    bool loadedOK() const;

    void setPriority( bool b = true );
    bool priority() const;

    bool operator<( const ImageRequest& other ) const;
    bool operator==( const ImageRequest& other ) const;

    virtual bool stillNeeded() const;

    bool doUpScale() const;
    void setUpScale( bool b );

private:
    bool _null;
    mutable QDeepCopy<QString> _fileName;
    mutable QMutex _fileNameLock;

    int _width;
    int _height;
    bool _cache;
    ImageClient* _client;
    int _angle;
    QSize _fullSize;
    bool _priority;
    bool _loadedOK;
    bool _dontUpScale;
};

}


#endif /* IMAGEREQUEST_H */

