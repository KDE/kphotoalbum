/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEMANAGER_PRELOADREQUEST_H
#define IMAGEMANAGER_PRELOADREQUEST_H
#include "ImageRequest.h"

namespace ImageManager
{
class ThumbnailCache;

class PreloadRequest : public ImageRequest
{
public:
    explicit PreloadRequest(const DB::FileName &fileName, const QSize &size, int angle, ImageClientInterface *client, const ThumbnailCache *thumbnailCache);
    bool stillNeeded() const override;

private:
    const ThumbnailCache *m_thumbnailCache;
};

}

#endif // IMAGEMANAGER_PRELOADREQUEST_H
// vi:expandtab:tabstop=4 shiftwidth=4:
