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

#ifndef IMAGEMANAGER_PRELOADREQUEST_H
#define IMAGEMANAGER_PRELOADREQUEST_H
#include "ImageRequest.h"

namespace ImageManager
{

class PreloadRequest : public ImageRequest
{
public:
    explicit PreloadRequest(const DB::FileName &fileName, const QSize &size, int angle, ImageClientInterface *client);
    bool stillNeeded() const override;
};

}

#endif // IMAGEMANAGER_PRELOADREQUEST_H
// vi:expandtab:tabstop=4 shiftwidth=4:
