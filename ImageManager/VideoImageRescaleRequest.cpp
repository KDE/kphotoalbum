/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "VideoImageRescaleRequest.h"

ImageManager::VideoImageRescaleRequest::VideoImageRescaleRequest(ImageRequest *originalRequest, const DB::FileName &path)
    : ImageRequest(originalRequest->databaseFileName(), originalRequest->size(), originalRequest->angle(), originalRequest->client())
    , m_originalRequest(originalRequest)
    , m_path(path)
{
    setIsThumbnailRequest(originalRequest->isThumbnailRequest());
}

ImageManager::VideoImageRescaleRequest::~VideoImageRescaleRequest()
{
    delete m_originalRequest;
}

DB::FileName ImageManager::VideoImageRescaleRequest::fileSystemFileName() const
{
    return m_path;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
