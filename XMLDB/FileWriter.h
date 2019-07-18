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
#ifndef XMLDB_FILEWRITER_H
#define XMLDB_FILEWRITER_H

#include <QRect>
#include <QString>

#include <DB/ImageInfoPtr.h>

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
    //void saveSettings(QXmlStreamWriter&);

private:
    Database *const m_db;
    QString areaToString(QRect area) const;
};

}

#endif /* XMLDB_FILEWRITER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
