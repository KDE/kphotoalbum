/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
#include "imagerequest.h"
#include <qmutex.h>
#include <qptrdict.h>

class ImageClient;

class ImageEvent :public QCustomEvent {
public:
    ImageEvent( ImageRequest* request, const QImage& image );
    ImageRequest* loadInfo();
    QImage image();

private:
    ImageRequest* _request;
    QImage _image;
};

// This class needs to inherit QObject to be capable of receiving events.
class ImageManager :public QObject {
    Q_OBJECT

public:
    enum StopAction { StopAll, StopOnlyNonPriorityLoads };

    void load(  ImageRequest* request );
    ImageRequest* next();
    static ImageManager* instance();
    void stop( ImageClient*, StopAction action = StopAll );

protected:
    virtual void customEvent( QCustomEvent* ev );

private:
    ImageManager();
    void init();
    static ImageManager* _instance;

    QValueList<ImageRequest*> _loadList;
    QWaitCondition _sleepers;
    QMutex _lock;
    QPtrDict<void> _clientList;
    ImageRequest* _currentLoading;
};

#endif /* IMAGEMANAGER_H */

