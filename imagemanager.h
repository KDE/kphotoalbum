/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H
#include "thumbnail.h"
#include "imageloader.h"
#include <qptrlist.h>
#include <qwaitcondition.h>
#include <qvaluelist.h>
#include <qevent.h>
#include <qdeepcopy.h>
#include <qstring.h>
#include <qpixmap.h>
class ImageClient;

class LoadInfo {
public:
    LoadInfo();
    LoadInfo( const QString& fileName, int width,  int heigth, int angle,
              ImageClient* client);

    bool isNull() const;
    QString fileName() const;
    int width() const;
    int height() const;
    int angle() const;

    void setCache( bool );
    bool cache() const;
    ImageClient* client();

    bool operator<( const LoadInfo& other ) const;
    bool operator==( const LoadInfo& other ) const;

private:
    bool _null;
    QDeepCopy<QString> _fileName;
    int _width;
    int _height;
    bool _cache;
    ImageClient* _client;
    int _angle;
};

class ImageEvent :public QCustomEvent {
public:
    ImageEvent( LoadInfo info, const QImage& image );
    LoadInfo loadInfo();
    QImage image();

private:
    LoadInfo _info;
    QDeepCopy<QImage> _image;
};

// This class needs to inherit QObject to be capable of receiving events.
class ImageManager :public QObject {
    Q_OBJECT

public:
    void load( const QString& fileName, ImageClient* client, int angle, int width, int height,
               bool cache, bool priority );
    LoadInfo next();
    static ImageManager* instance();
    void stop( ImageClient* );

protected:
    virtual void customEvent( QCustomEvent* ev );

private:
    ImageManager();
    void init();
    static ImageManager* _instance;

    ImageLoader* _imageLoader;
    QValueList<LoadInfo> _loadList;
    QWaitCondition* _sleepers;
    QMutex* _lock;
    QMap<LoadInfo, ImageClient*> _clientMap;
};

#endif /* IMAGEMANAGER_H */

