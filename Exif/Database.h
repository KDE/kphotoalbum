/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef EXIFDATABASE_H
#define EXIFDATABASE_H

#include <QSqlDatabase>
#include <qstring.h>
#include <QList>
#include <qpair.h>
#include <DB/FileName.h>

namespace Exiv2 { class ExifData; }

typedef QPair<int,int> Rational;
typedef QList<Rational> RationalList;

namespace Exif
{
class DatabaseElement;

// ============================================================================
// IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT
// ============================================================================
//
// It is the resposibility of the methods in here to bail out in case database
// support is not available ( !isAvailable() ). This is to simplify client code.
class Database {

public:
    typedef QList<DatabaseElement*> ElementList;
    typedef QPair<QString, QString> Camera;
    typedef QList<Camera> CameraList;
    typedef QString Lens;
    typedef QList<Lens> LensList;

    static Database* instance();
    static void deleteInstance();
    static bool isAvailable();
    /**
     * @brief DBVersion is the exif search database schema version currently supported by KPhotoAlbum.
     * @return the Exif Database version
     */
    static constexpr int DBVersion();

    bool isOpen() const;
    bool isUsable() const;
    /**
     * @brief DBFileVersion is the database schema version used in the exif-info.db file.
     * @return the database schema version used by the database file, or 0 on error.
     */
    int DBFileVersion() const;
    /**
     * @brief DBFileVersionGuaranteed reflects DBVersion of the last time the exif db has been built.
     * It is just like the DBFileVersion() but concerning the data.
     * The schema version is automatically updated to a newer schema, but normally the
     * data in the exif database is not.
     * In this situation, only newly added pictures are populated with the new fields, whereas
     * existing pictures have empty values.
     * However, once the user rebuilds the exif database, we can guarantee all entries in the
     * database to conform to the new schema, and DBFileVersionGuaranteed() will be updated to the new value.
     * @return 0 <= DBFileVersionGuaranteed() <= DBFileVersion()
     */
    int DBFileVersionGuaranteed() const;
    bool add( const DB::FileName& fileName );
    void remove( const DB::FileName& fileName );
    bool readFields( const DB::FileName& fileName, ElementList &fields) const;
    DB::FileNameSet filesMatchingQuery( const QString& query ) const;
    CameraList cameras() const;
    LensList lenses() const;
    void recreate();

protected:
    enum DBSchemaChangeType { SchemaChanged, SchemaAndDataChanged };
    static QString exifDBFile();
    void openDatabase();
    void populateDatabase();
    void updateDatabase();
    void createMetadataTable(DBSchemaChangeType change);
    static QString connectionName();
    void insert( const DB::FileName& filename, Exiv2::ExifData );

private:
    bool m_isOpen;
    bool m_doUTF8Conversion;
    Database();
    ~Database();
    void init();
    static Database* s_instance;
    QSqlDatabase m_db;
};

}

#endif /* EXIFDATABASE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
