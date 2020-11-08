/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
