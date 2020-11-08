/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEMANAGER_ASYNCLOADER_H
#define IMAGEMANAGER_ASYNCLOADER_H

#include "RequestQueue.h"
#include "enums.h"

#include <MainWindow/Window.h>

#include <QImage>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

class QEvent;

namespace ImageManager
{

class ImageRequest;
class ImageClientInterface;
class ImageLoaderThread;

// This class needs to inherit QObject to be capable of receiving events.
class AsyncLoader : public QObject
{
    Q_OBJECT

public:
    static AsyncLoader *instance();

    // Request to load an image. The Manager takes over the ownership of
    // the request (and may delete it anytime).
    bool load(ImageRequest *request);

    // Stop loading all images requested by the given client.
    void stop(ImageClientInterface *, StopAction action = StopAll);
    int activeCount() const;
    bool isExiting() const;

protected:
    void customEvent(QEvent *ev) override;
    bool loadVideo(ImageRequest *);
    void loadImage(ImageRequest *);

private:
    friend class ImageLoaderThread; // may call 'next()'
    friend class MainWindow::Window; // may call 'requestExit()'
    void init();

    ImageRequest *next();

    void requestExit();

    static AsyncLoader *s_instance;

    RequestQueue m_loadList;
    QWaitCondition m_sleepers;
    // m_lock protects m_loadList and m_currentLoading
    mutable QMutex m_lock;
    QSet<ImageRequest *> m_currentLoading;
    QImage m_brokenImage;
    QList<ImageLoaderThread *> m_threadList;
    bool m_exitRequested;
    int m_exitRequestsProcessed;
};
}

#endif /* IMAGEMANAGER_ASYNCLOADER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
