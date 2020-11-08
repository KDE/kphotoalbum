/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CancelEvent.h"

#include "ImageRequest.h"
ImageManager::CancelEvent::CancelEvent(ImageRequest *request)
    : QEvent(static_cast<QEvent::Type>(CANCELEVENTID))
    , m_request(request)
{
}

ImageManager::CancelEvent::~CancelEvent()
{
    delete m_request;
}

ImageManager::ImageRequest *ImageManager::CancelEvent::request() const
{
    return m_request;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
