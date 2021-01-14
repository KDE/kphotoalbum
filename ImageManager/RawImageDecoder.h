/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RAWIMAGEDECODER_H
#define RAWIMAGEDECODER_H

#include "ImageDecoder.h"

#include <kpabase/FileName.h>

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
