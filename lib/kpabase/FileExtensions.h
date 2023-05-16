// SPDX-FileCopyrightText: 2006 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2003-2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007-2011 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef KPABASE_FILE_EXTENSIONS_H
#define KPABASE_FILE_EXTENSIONS_H
#include <kpabase/FileName.h>

#include <QSet>
#include <QString>

namespace KPABase
{

enum class FileTypePreference {
    NoPreference, ///< Just check the file type
    PreferNonRawFile ///< Check if a non-raw image file exists before considering the file type.
};

/**
 * @brief fileCanBeSkipped determines whether the file is of interest to KPhotoAlbum or not.
 * It takes into account known skippable suffixes (e.g. thumbnail files) and the
 * configuration on when to skip raw files (i.e. `skipRawIfOtherMatches`).
 *
 * @param loadedFiles a set of file names that are already loaded.
 * @param imageFile the image file
 * @return \c true, if the file can be skipped, \c false otherwise.
 */
bool fileCanBeSkipped(const DB::FileNameSet &loadedFiles, const DB::FileName &imageFile);

/**
 * @brief isUsableRawImage checks if the filename is a raw file by checking its extension.
 * If a preference for non-raw image files is given, check if a "normal" image file exists before considering the raw file.
 *
 * @param imageFile
 * @param preference FileTypePreference::NoPreference to check the file type only, or FileTypePreference::PreferNonRawFile to return \c false if a non-raw file exists
 * @return \c true, if \c imageFile is a raw file (and no preferrable non-raw image exists), \c false otherwise.
 */
bool isUsableRawImage(const DB::FileName &imageFile, FileTypePreference preference = FileTypePreference::NoPreference);

/**
 * @brief isVideo checks if the filename is a video file by checking its extension.
 * @param fileName
 * @return \c true, if the file extension matches one of the known video extensions.
 */
bool isVideo(const DB::FileName &fileName);

QStringList rawExtensions();
const QSet<QString> &videoExtensions();

}

#endif /* KPABASE_FILE_EXTENSIONS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
