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

#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H
#include <qwaitcondition.h>
#include <QList>
#include <qevent.h>
#include <qmutex.h>
#include <qimage.h>
#include "RequestQueue.h"
#include "StopAction.h"

namespace ImageManager
{

class ImageRequest;
class ImageClient;
class ImageLoader;

// This class needs to inherit QObject to be capable of receiving events.
class Manager :public QObject {
    Q_OBJECT

public:
    static Manager* instance();

    // Request to load an image. The Manager takes over the ownership of
    // the request.
    void load( ImageRequest* request );

    // Stop loading all images requested by the given client.
    void stop( ImageClient*, StopAction action = StopAll );

protected:
    virtual void customEvent( QEvent* ev );
    void loadVideo( ImageRequest* );
    void loadImage( ImageRequest* );

private:
    friend class ImageLoader;  // may call 'next()'
    void init();

    ImageRequest* next();

    static Manager* _instance;

    RequestQueue _loadList;
    QWaitCondition _sleepers;
    QMutex _lock;
    QSet<ImageRequest*> _currentLoading;
};

}

#endif /* IMAGEMANAGER_H */

