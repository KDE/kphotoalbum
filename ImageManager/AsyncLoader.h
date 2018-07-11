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

#ifndef IMAGEMANAGER_ASYNCLOADER_H
#define IMAGEMANAGER_ASYNCLOADER_H

#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QList>

#include "enums.h"
#include "RequestQueue.h"
#include "MainWindow/Window.h"

class QEvent;

namespace ImageManager
{

class ImageRequest;
class ImageClientInterface;
class ImageLoaderThread;

// This class needs to inherit QObject to be capable of receiving events.
class AsyncLoader :public QObject {
    Q_OBJECT

public:
    static AsyncLoader* instance();

    // Request to load an image. The Manager takes over the ownership of
    // the request (and may delete it anytime).
    bool load( ImageRequest* request );

    // Stop loading all images requested by the given client.
    void stop( ImageClientInterface*, StopAction action = StopAll );
    int activeCount() const;
    bool isExiting() const;

protected:
    virtual void customEvent( QEvent* ev );
    bool loadVideo( ImageRequest* );
    void loadImage( ImageRequest* );

private:
    friend class ImageLoaderThread;  // may call 'next()'
    friend class MainWindow::Window; // may call 'requestExit()'
    void init();

    ImageRequest* next();

    void requestExit();

    static AsyncLoader* s_instance;

    RequestQueue m_loadList;
    QWaitCondition m_sleepers;
    // m_lock protects m_loadList and m_currentLoading
    mutable QMutex m_lock;
    QSet<ImageRequest*> m_currentLoading;
    QImage m_brokenImage;
    QList<ImageLoaderThread*> m_threadList;
    bool m_exitRequested;
    int m_exitRequestsProcessed;
};

}

#endif /* IMAGEMANAGER_ASYNCLOADER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
