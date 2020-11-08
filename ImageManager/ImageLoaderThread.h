/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGELOADERTHREAD_H
#define IMAGELOADERTHREAD_H

#include <QImage>
#include <qthread.h>

namespace ImageManager
{
class AsyncLoader;
class ImageRequest;
class ThumbnailStorage;

static const int maxJPEGMemorySize = (20 * 1024 * 1024);

class ImageLoaderThread : public QThread
{
public:
    ImageLoaderThread(size_t bufsize = maxJPEGMemorySize);
    ~ImageLoaderThread() override;

protected:
    void run() override;
    QImage loadImage(ImageRequest *request, bool &ok);
    static int calcLoadSize(ImageRequest *request);
    QImage scaleAndRotate(ImageRequest *request, QImage img);
    bool shouldImageBeScale(const QImage &img, ImageRequest *request);

private:
    char *m_imageLoadBuffer;
    size_t m_bufSize;
};
}

#endif /* IMAGELOADERTHREAD_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
