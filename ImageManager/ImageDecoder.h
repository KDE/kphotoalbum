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
#ifndef IMAGEDECODER_H
#define IMAGEDECODER_H

#include <QList>

class QSize;
class QImage;

namespace DB
{
class FileName;
}

namespace ImageManager
{
class ImageRequest;
class ImageDecoder
{
public:
    static bool decode(QImage *img, ImageRequest *request, QSize *fullSize, int dim = -1);
    static bool mightDecode(const DB::FileName &imageFile);

    virtual ~ImageDecoder();

protected:
    ImageDecoder();
    virtual bool _decode(QImage *img, ImageRequest *request, QSize *fullSize, int dim = -1) = 0;
    virtual bool _mightDecode(const DB::FileName &imageFile) = 0;

private:
    static QList<ImageDecoder *> *decoders();
};
}

#endif /* IMAGEDECODER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
