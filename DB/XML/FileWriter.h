// SPDX-FileCopyrightText: 2006-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2009 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef XMLDB_FILEWRITER_H
#define XMLDB_FILEWRITER_H

#include <DB/ImageInfoPtr.h>

#include <QRect>
#include <QString>

class QXmlStreamWriter;

namespace DB
{
class ImageDB;

class Database;

class FileWriter
{
public:
    explicit FileWriter(DB::ImageDB *db)
        : m_db(db)
    {
    }
    void save(const QString &fileName, bool isAutoSave);

protected:
    void saveCategories(QXmlStreamWriter &);
    void saveImages(QXmlStreamWriter &);
    void saveBlockList(QXmlStreamWriter &);
    void saveMemberGroups(QXmlStreamWriter &);
    void saveGlobalSortOrder(QXmlStreamWriter &);
    void save(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info);
    void writeCategories(QXmlStreamWriter &, const DB::ImageInfoPtr &info);
    void writeCategoriesCompressed(QXmlStreamWriter &, const DB::ImageInfoPtr &info);
    bool shouldSaveCategory(const QString &categoryName) const;
    // void saveSettings(QXmlStreamWriter&);

private:
    DB::ImageDB *const m_db;
    QString areaToString(QRect area) const;
};

}

#endif /* XMLDB_FILEWRITER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
