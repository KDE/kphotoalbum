/* SPDX-FileCopyrightText: 2020 The KPhotoAlbum development team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KPA_THUMBNAILTOOL_THUMBNAILCACHECONVERTER_H
#define KPA_THUMBNAILTOOL_THUMBNAILCACHECONVERTER_H

class QIODevice;
class QString;
class QTextStream;

namespace KPAThumbnailTool
{
/**
 * @brief Convert a version 5 ThumbnailCache index file to version 4.
 * The old thumbnailindex file is backed up with a suffic '.bak'.
 *
 * This function does not use the ThumbnailCache code at all,
 * but just reads the file and converts the header accordingly.
 *
 * This way, code in ThumbnailCache is not bloated by a niche-usecase.
 *
 * @param indexFilename
 * @param err an output stream for error messages
 * @return 0 on success, 1 otherwise.
 */
int convertV5ToV4Cache(const QString &indexFilename, QTextStream &err);

/**
 * @brief Convert a version 5 ThumbnailCache index file to version 4.
 * This function does not use the ThumbnailCache code at all,
 * but just reads the file and converts the header accordingly.
 *
 * This way, code in ThumbnailCache is not bloated by a niche-usecase.
 * @param fromFile the (already opened) QIODevice containing the old index data
 * @param toFile an open QIODevice to write to
 * @param err a stream for error messages.
 * @return \c true on success, \c false if the input file is not valid.
 */
bool convertV5ToV4Cache(QIODevice &fromFile, QIODevice &toFile, QTextStream &err);
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
