/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ThumbnailRequest.h"

#include "ThumbnailModel.h"

ThumbnailView::ThumbnailRequest::ThumbnailRequest(int row, const DB::FileName &fileName, const QSize &size, int angle, ThumbnailModel *client)
    : ImageManager::ImageRequest(fileName, size, angle, client)
    , m_thumbnailModel(client)
    , m_row(row)
{
    setIsThumbnailRequest(true);
}

bool ThumbnailView::ThumbnailRequest::stillNeeded() const
{
    return m_thumbnailModel->thumbnailStillNeeded(m_row);
}
// vi:expandtab:tabstop=4 shiftwidth=4:
