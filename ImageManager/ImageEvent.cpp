/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageEvent.h"

ImageManager::ImageEvent::ImageEvent(ImageRequest *request, const QImage &image)
    : QEvent(static_cast<QEvent::Type>(ImageEventID))
    , m_request(request)
    , m_image(image)
{
    // PENDING(blackie): Investigate if this is still needed with Qt4.
    // We would like to use QDeepCopy, but that results in multiple
    // individual instances on the GUI thread, which is kind of real bad
    // when  the image is like 40Mb large.
    m_image.detach();
}

ImageManager::ImageRequest *ImageManager::ImageEvent::loadInfo()
{
    return m_request;
}

QImage ImageManager::ImageEvent::image()
{
    return m_image;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
