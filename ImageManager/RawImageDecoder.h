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
#ifndef RAWIMAGEDECODER_H
#define RAWIMAGEDECODER_H

#include "ImageDecoder.h"

#include <DB/FileName.h>

#include <QStringList>

namespace ImageManager
{

class RAWImageDecoder : public ImageDecoder
{
public:
    /**
     * @brief fileCanBeSkipped determines whether the file is of interest to KPhotoAlbum or not.
     * It takes into account known skippable suffixes (e.g. thumbnail files) and the
     * configuration on when to skip raw files (i.e. `skipRawIfOtherMatches`).
     *
     * @param loadedFiles a set of file names that are already loaded.
     * @param imageFile the image file
     * @return \c true, if the file can be skipped, \c false otherwise.
     */
    bool fileCanBeSkipped(const DB::FileNameSet &loadedFiles, const DB::FileName &imageFile) const;
    static bool isRAW(const DB::FileName &imageFile);
    static QStringList rawExtensions();

protected:
    bool _decode(QImage *img, ImageRequest *request, QSize *fullSize, int dim = -1) override;
    bool _mightDecode(const DB::FileName &imageFile) override;
};
}

#endif /* RAWIMAGEDECODER_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
