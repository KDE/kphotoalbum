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

#include <DB/FileNameList.h>
#include <DB/FileInfo.h>

#include <QList>
#include <QPair>
#include <QSqlDatabase>
#include <QString>

namespace Exiv2 { class ExifData; }

typedef QPair<int,int> Rational;
typedef QList<Rational> RationalList;
typedef QPair<DB::FileName, Exiv2::ExifData> DBExifInfo;

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
    /**
     * @brief add a file and its exif data to the database.
     * If the file already exists in the database, the new data replaces the existing data.
     * @param fileInfo the file
     * @return
     */
    bool add( DB::FileInfo& fileInfo );
    bool add( const DB::FileName& fileName );
    bool add( const DB::FileNameList& list );
    void remove( const DB::FileName& fileName );
    void remove( const DB::FileNameList& list );
    /**
     * @brief readFields searches the exif database for a given file and fills the element list with values.
     * If the query fails or has no result, the ElementList is not changed.
     * @param fileName
     * @param fields a list of the DatabaseElements that you want to read.
     * @return true, if the fileName is found in the database, false otherwise.
     */
    bool readFields( const DB::FileName& fileName, ElementList &fields) const;
    DB::FileNameSet filesMatchingQuery( const QString& query ) const;
    CameraList cameras() const;
    LensList lenses() const;
    void recreate();
    bool startInsertTransaction();
    bool commitInsertTransaction();
    bool abortInsertTransaction();

protected:
    enum DBSchemaChangeType { SchemaChanged, SchemaAndDataChanged };
    static QString exifDBFile();
    void openDatabase();
    void populateDatabase();
    void updateDatabase();
    void createMetadataTable(DBSchemaChangeType change);
    static QString connectionName();
    bool insert( const DB::FileName& filename, Exiv2::ExifData );
    bool insert( QList<DBExifInfo> );

private:
    void showErrorAndFail( QSqlQuery &query ) const;
    void showErrorAndFail(const QString &errorMessage , const QString &technicalInfo) const;
    bool m_isOpen;
    bool m_doUTF8Conversion;
    mutable bool m_isFailed;
    Database();
    ~Database();
    void init();
    QSqlQuery *getInsertQuery();
    void concludeInsertQuery(QSqlQuery *);
    static Database* s_instance;
    QString m_queryString;
    QSqlDatabase m_db;
    QSqlQuery *m_insertTransaction;
};

}

#endif /* EXIFDATABASE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
