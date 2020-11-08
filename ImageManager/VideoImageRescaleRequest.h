/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIDEOIMAGERESCALEREQUEST_H
#define VIDEOIMAGERESCALEREQUEST_H

#include "ImageClientInterface.h"
#include "ImageRequest.h"

namespace ImageManager
{

class VideoImageRescaleRequest : public ImageRequest
{
public:
    VideoImageRescaleRequest(ImageRequest *originalRequest, const DB::FileName &path);
    ~VideoImageRescaleRequest() override;
    DB::FileName fileSystemFileName() const override;

private:
    ImageRequest *m_originalRequest;
    DB::FileName m_path;
};

}

#endif // VIDEOIMAGERESCALEREQUEST_H
// vi:expandtab:tabstop=4 shiftwidth=4:
