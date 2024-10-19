// SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>
// SPDX-FileCopyrightText: 2003-2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2005 Steffen Hansen <hansen@kde.org>
// SPDX-FileCopyrightText: 2005-2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2006-2007 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007-2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2018 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageLoaderThread.h"

#include "AsyncLoader.h"
#include "ImageDecoder.h"
#include "ImageEvent.h"
#include "RawImageDecoder.h"

#include <Utilities/FastJpeg.h>
#include <kpabase/ImageUtil.h>
#include <kpabase/Logging.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <QApplication>
#include <QFileInfo>
#include <QLoggingCategory>

namespace ImageManager
{
// Create a global instance. Its constructor will itself register it.
RAWImageDecoder rawdecoder;
}

ImageManager::ImageLoaderThread::ImageLoaderThread(size_t bufsize)
    : m_imageLoadBuffer(new char[bufsize])
    , m_bufSize(bufsize)
{
}

ImageManager::ImageLoaderThread::~ImageLoaderThread()
{
    delete[] m_imageLoadBuffer;
}

void ImageManager::ImageLoaderThread::run()
{
    while (true) {
        ImageRequest *request = AsyncLoader::instance()->next();
        Q_ASSERT(request);
        if (request->isExitRequest()) {
            return;
        }
        bool ok;

        QImage img = loadImage(request, ok);

        if (ok) {
            img = scaleAndRotate(request, img);
        }

        request->setLoadedOK(ok);
        ImageEvent *iew = new ImageEvent(request, img);
        QApplication::postEvent(AsyncLoader::instance(), iew);
    }
}

QImage ImageManager::ImageLoaderThread::loadImage(ImageRequest *request, bool &ok)
{
    int dim = calcLoadSize(request);
    QSize fullSize;

    ok = false;
    if (!request->fileSystemFileName().exists())
        return QImage();

    QImage img;
    if (Utilities::isJPEG(request->fileSystemFileName())) {
        ok = Utilities::loadJPEG(&img, request->fileSystemFileName(), &fullSize, dim,
                                 m_imageLoadBuffer, m_bufSize);
        if (ok == true)
            request->setFullSize(fullSize);
    }

    else {
        // At first, we have to give our RAW decoders a try. If we allowed
        // QImage's load() method, it'd for example load a tiny thumbnail from
        // NEF files, which is not what we want.
        ok = ImageDecoder::decode(&img, request, &fullSize, dim);
        if (ok)
            request->setFullSize(img.size());
    }

    if (!ok) {
        // Now we can try QImage's stuff as a fallback...
        ok = img.load(request->fileSystemFileName().absolute());
        if (ok)
            request->setFullSize(img.size());
    }

    return img;
}

int ImageManager::ImageLoaderThread::calcLoadSize(ImageRequest *request)
{
    return qMax(request->width(), request->height());
}

QImage ImageManager::ImageLoaderThread::scaleAndRotate(ImageRequest *request, QImage img)
{
    if (request->angle() != 0 && !request->imageIsPreRotated()) {
        QTransform matrix;
        matrix.rotate(request->angle());
        img = img.transformed(matrix);
        int angle = (request->angle() + 360) % 360;
        Q_ASSERT(angle >= 0 && angle <= 360);
        qCDebug(ImageManagerLog) << "Rotating image to" << angle << "degrees:" << request->fileSystemFileName().relative();
        if (angle == 90 || angle == 270)
            request->setFullSize(QSize(request->fullSize().height(), request->fullSize().width()));
    }

    // If we are looking for a scaled version, then scale
    if (shouldImageBeScale(img, request))
        img = Utilities::scaleImage(img, request->size(), Qt::KeepAspectRatio);

    return img;
}

bool ImageManager::ImageLoaderThread::shouldImageBeScale(const QImage &img, ImageRequest *request)
{
    // No size specified, meaning we want it full size.
    if (request->width() == -1)
        return false;

    if (img.width() < request->width() && img.height() < request->height()) {
        // The image is smaller than the requets.
        return request->doUpScale();
    }

    return true;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
