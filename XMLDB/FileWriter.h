/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef XMLDB_FILEWRITER_H
#define XMLDB_FILEWRITER_H

#include <DB/ImageInfoPtr.h>

#include <QRect>
#include <QString>

class QXmlStreamWriter;

namespace XMLDB
{
class Database;

class FileWriter
{
public:
    explicit FileWriter(Database *db)
        : m_db(db)
    {
    }
    void save(const QString &fileName, bool isAutoSave);
    static QString escape(const QString &);

protected:
    void saveCategories(QXmlStreamWriter &);
    void saveImages(QXmlStreamWriter &);
    void saveBlockList(QXmlStreamWriter &);
    void saveMemberGroups(QXmlStreamWriter &);
    void save(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info);
    void writeCategories(QXmlStreamWriter &, const DB::ImageInfoPtr &info);
    void writeCategoriesCompressed(QXmlStreamWriter &, const DB::ImageInfoPtr &info);
    bool shouldSaveCategory(const QString &categoryName) const;
    // void saveSettings(QXmlStreamWriter&);

private:
    Database *const m_db;
    QString areaToString(QRect area) const;
};

}

#endif /* XMLDB_FILEWRITER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
