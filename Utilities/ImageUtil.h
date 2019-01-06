/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H
#include "DB/FileName.h"

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
QImage scaleImage(const QImage &image, const QSize& size, Qt::AspectRatioMode mode=Qt::IgnoreAspectRatio );

/**
 * @brief saveImage saves a QImage to a FileName, making sure that the directory exists.
 * @param fileName
 * @param image
 * @param format the storage format for QImage::save(), usually "JPEG"
 */
void saveImage( const DB::FileName& fileName, const QImage& image, const char* format );
}


#endif /* IMAGE_UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
