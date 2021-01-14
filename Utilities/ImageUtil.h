/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H
#include <kpabase/FileName.h>

#include <QImage>

namespace Utilities
{
/**
 * @brief scaleImage returns the scaled image, honoring the settings for smooth scaling.
 * @param image
 * @param size
 * @param mode aspect ratio mode
 * @return a scaled image
 */
QImage scaleImage(const QImage &image, const QSize &size, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio);

/**
 * @brief saveImage saves a QImage to a FileName, making sure that the directory exists.
 * @param fileName
 * @param image
 * @param format the storage format for QImage::save(), usually "JPEG"
 */
void saveImage(const DB::FileName &fileName, const QImage &image, const char *format);
}

#endif /* IMAGE_UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
