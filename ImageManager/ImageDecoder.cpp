/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
#include "ImageDecoder.h"

#include <DB/FileName.h>

QList<ImageManager::ImageDecoder *> *ImageManager::ImageDecoder::decoders()
{
    static QList<ImageDecoder *> s_decoders;
    return &s_decoders;
}

ImageManager::ImageDecoder::ImageDecoder()
{
    decoders()->append(this);
}

ImageManager::ImageDecoder::~ImageDecoder()
{
    decoders()->removeOne(this);
}

bool ImageManager::ImageDecoder::decode(QImage *img, ImageRequest *request, QSize *fullSize, int dim)
{
    for (ImageDecoder *decoder : *decoders()) {
        if (decoder->_decode(img, request, fullSize, dim))
            return true;
    }
    return false;
}

bool ImageManager::ImageDecoder::mightDecode(const DB::FileName &imageFile)
{
    for (ImageDecoder *decoder : *decoders()) {
        if (decoder->_mightDecode(imageFile))
            return true;
    }
    return false;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
