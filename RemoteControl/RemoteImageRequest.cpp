/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#include "RemoteImageRequest.h"

namespace RemoteControl
{

RemoteImageRequest::RemoteImageRequest(const DB::FileName &fileName, const QSize &size, int angle, ViewType type, RemoteInterface *client)
    : ImageManager::ImageRequest(fileName, size, angle, client)
    , m_interface(client)
    , m_type(type)
{
}

bool RemoteImageRequest::stillNeeded() const
{
    return m_interface->requestStillNeeded(fileSystemFileName());
}

ViewType RemoteImageRequest::type() const
{
    return m_type;
}

} // namespace RemoteControl
