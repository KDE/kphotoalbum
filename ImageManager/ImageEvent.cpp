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

#include "ImageEvent.h"

ImageManager::ImageEvent::ImageEvent( ImageRequest* request, const QImage& image )
    : QEvent( static_cast<QEvent::Type>(ImageEventID) ), _request( request ),  _image( image )
{
    // PENDING(blackie): Investigate if this is still needed with Qt4.
    // We would like to use QDeepCopy, but that results in multiple
    // individual instances on the GUI thread, which is kind of real bad
    // when  the image is like 40Mb large.
    _image.detach();
}

ImageManager::ImageRequest* ImageManager::ImageEvent::loadInfo()
{
    return _request;
}

QImage ImageManager::ImageEvent::image()
{
    return _image;
}
