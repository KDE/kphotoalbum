/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef THUMBNAILREQUEST_H
#define THUMBNAILREQUEST_H
#include <ImageManager/ImageRequest.h>

namespace ThumbnailView
{
class ThumbnailModel;

class ThumbnailRequest : public ImageManager::ImageRequest
{
public:
    ThumbnailRequest(int row, const DB::FileName &fileName, const QSize &size, int angle, ThumbnailModel *client);
    bool stillNeeded() const override;

private:
    const ThumbnailModel *const m_thumbnailModel;
    int m_row;
};

}

#endif /* THUMBNAILREQUEST_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
