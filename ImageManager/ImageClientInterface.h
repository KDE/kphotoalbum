/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGECLIENTINTERFACE_H
#define IMAGECLIENTINTERFACE_H
class QSize;
class QImage;
class QString;

namespace DB
{
class FileName;
}

namespace ImageManager
{
class ImageRequest;

/**
 * An ImageClient is part of the ImageRequest and is called back when
 * an image has been loaded.
 */
class ImageClientInterface
{
public:
    virtual ~ImageClientInterface();

    /**
     * Callback on loaded image.
     */
    virtual void pixmapLoaded(ImageRequest *request, const QImage &image) = 0;
    virtual void requestCanceled() { }
};
}

#endif /* IMAGECLIENTINTERFACE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
