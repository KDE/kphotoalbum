/* Copyright (C) 2003-2011 Jesper K. Pedersen <blackie@kde.org>

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
