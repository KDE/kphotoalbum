/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECONTROL_REMOTEIMAGEREQUEST_H
#define REMOTECONTROL_REMOTEIMAGEREQUEST_H

#include "RemoteInterface.h"
#include "Types.h"

#include <ImageManager/ImageRequest.h>

namespace RemoteControl
{

class RemoteImageRequest : public ImageManager::ImageRequest
{
public:
    RemoteImageRequest(const DB::FileName &fileName, const QSize &size, int angle, ViewType type, RemoteInterface *client);
    bool stillNeeded() const override;
    ViewType type() const;

private:
    RemoteInterface *m_interface;
    ViewType m_type;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_REMOTEIMAGEREQUEST_H
