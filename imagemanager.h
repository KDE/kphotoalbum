/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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
#include <qcache.h>

class ImageClient;

class LoadInfo {
public:
    LoadInfo();
    LoadInfo( const QString& fileName, int width,  int heigth, int angle,
              bool priority, ImageClient* client);

    bool isNull() const;
    QString fileName() const;
    int width() const;
    int height() const;
    int angle() const;

    void setCache( bool );
    bool cache() const;
    ImageClient* client();

    QSize fullSize() const;
    void setFullSize( const QSize& );
    void setLoadedOK( bool ok );
    bool loadedOK() const;

    bool priority() const;

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
    QSize _fullSize;
    bool _priority;
    bool _loadedOK;
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
    enum StopAction { StopAll, StopOnlyNonPriorityLoads };

    void load( const QString& fileName, ImageClient* client, int angle, int width, int height,
               bool cache, bool priority );
    LoadInfo next();
    static ImageManager* instance();
    void stop( ImageClient*, StopAction action = StopAll );

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
    LoadInfo _currentLoading;
};

#endif /* IMAGEMANAGER_H */

