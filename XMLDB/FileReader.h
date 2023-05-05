// SPDX-FileCopyrightText: 2006-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2008 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2009 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2015 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef XMLDB_FILEREADER_H
#define XMLDB_FILEREADER_H

#include "XmlReader.h"

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>

#include <QSharedPointer>

class QXmlStreamReader;

namespace DB
{
class ImageDB;
}
namespace XMLDB
{
class FileReader
{

public:
    FileReader(DB::ImageDB *db)
        : m_db(db)
        , m_fileVersion(0)
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
    // void loadSettings(ReaderPtr reader);

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
    DB::ImageDB *const m_db;
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
