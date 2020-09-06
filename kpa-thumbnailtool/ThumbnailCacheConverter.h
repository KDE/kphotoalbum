/* Copyright (C) 2020 The KPhotoAlbum development team

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
