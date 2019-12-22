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
#ifndef XMLDB_FILEREADER_H
#define XMLDB_FILEREADER_H

#include "XmlReader.h"

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>

#include <QSharedPointer>

class QXmlStreamReader;

namespace XMLDB
{
class Database;

class FileReader
{

public:
    FileReader(Database *db)
        : m_db(db)
        , m_nextStackId(1)
    {
    }
    void read(const QString &configFile);
    static QString unescape(const QString &);
    DB::StackID nextStackId() const { return m_nextStackId; }

protected:
    void loadCategories(ReaderPtr reader);
    void loadImages(ReaderPtr reader);
    void loadBlockList(ReaderPtr reader);
    void loadMemberGroups(ReaderPtr reader);
    //void loadSettings(ReaderPtr reader);

    DB::ImageInfoPtr load(const DB::FileName &filename, ReaderPtr reader);
    ReaderPtr readConfigFile(const QString &configFile);

    void createSpecialCategories();

    void checkIfImagesAreSorted();
    void checkIfAllImagesHaveSizeAttributes();

    /**
     * @brief Repair the database if an issue was flagged for repair.
     * DB repairs that only require local knowledge are usually done in the respective
     * load* method. This method is called after the database file was loaded.
     *
     * Currently, this tries to fix the following issues:
     *
     * - Bug #415415 - Renaming tag groups can produce tags with id=0
     */
    void repairDB();

private:
    Database *const m_db;
    int m_fileVersion;
    DB::StackID m_nextStackId;

    // During profilation I found that it was rather expensive to look this up over and over again (once for each image)
    DB::CategoryPtr m_folderCategory;
    /// Flag indicating that repair is necessary
    bool m_repairTagsWithNullIds = false;
};

}

#endif /* XMLDB_FILEREADER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
