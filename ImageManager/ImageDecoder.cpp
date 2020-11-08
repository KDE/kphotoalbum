/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
