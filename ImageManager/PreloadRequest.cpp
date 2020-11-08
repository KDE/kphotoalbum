/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PreloadRequest.h"

#include "ThumbnailCache.h"

ImageManager::PreloadRequest::PreloadRequest(const DB::FileName &fileName, const QSize &size, int angle, ImageClientInterface *client, const ThumbnailCache *thumbnailCache)
    : ImageRequest(fileName, size, angle, client)
    , m_thumbnailCache(thumbnailCache)
{
}

bool ImageManager::PreloadRequest::stillNeeded() const
{
    return !m_thumbnailCache->contains(databaseFileName());
}
// vi:expandtab:tabstop=4 shiftwidth=4:
