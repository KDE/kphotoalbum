// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIFDATABASE_H
#define EXIFDATABASE_H

#include <kpabase/FileNameList.h>

#include <QList>
#include <QPair>
#include <QString>

namespace DB
{
class UIDelegate;
class AbstractProgressIndicator;
}
namespace Exiv2
{
class ExifData;
}

typedef QPair<int, int> Rational;
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
class Database
{
public:
    typedef QList<DatabaseElement *> ElementList;
    typedef QPair<QString, QString> Camera;
    typedef QList<Camera> CameraList;
    typedef QString Lens;
    typedef QList<Lens> LensList;

    Database(const QString &sqliteFileName, DB::UIDelegate &uiDelegate);
    Database(const Database &) = delete;
    ~Database();

    Database &operator=(const Database &) = delete;

    static bool isAvailable();
    /**
     * @brief DBVersion is the exif search database schema version currently supported by KPhotoAlbum.
     * @return the Exif Database version
     */
    static int DBVersion();

    /**
     * @brief isUsable
     * @return \c true, if the Exif database is available, open, and not in a failure state. \c false otherwise
     */
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
     * @brief Adds a file and its exif data to the exif database.
     * If the file already exists in the database, the new data replaces the existing data.
     * @param filename
     * @param data the exif data
     * @return \c true, if the operation succeeded, \c false otherwise
     */
    bool add(const DB::FileName &filename, Exiv2::ExifData data);
    /**
     * @brief Adds a file to the exif database, reading its exif data from the file.
     * @param fileName
     * @return \c true, if the operation succeeded, \c false otherwise
     */
    bool add(const DB::FileName &fileName);
    /**
     * @brief Adds a list of files to the exif database, reading the exif data from the files.
     * @param list
     * @return \c true, if the operation succeeded, \c false otherwise
     */
    bool add(const DB::FileNameList &list);
    /**
     * @brief Removes a single file from the exif database.
     * Removing a file that is not in the database is allowed.
     * @param fileName
     */
    void remove(const DB::FileName &fileName);
    /**
     * @brief Removes a list of files from the exif database.
     * Passing an empty list or a list that contains files that are not actually in the exif database is allowed.
     * @param list
     */
    void remove(const DB::FileNameList &list);
    /**
     * @brief readFields searches the exif database for a given file and fills the element list with values.
     * If the query fails or has no result, the ElementList is not changed.
     * @param fileName
     * @param fields a list of the DatabaseElements that you want to read.
     * @return true, if the fileName is found in the database, false otherwise.
     */
    bool readFields(const DB::FileName &fileName, ElementList &fields) const;
    DB::FileNameSet filesMatchingQuery(const QString &query) const;
    CameraList cameras() const;
    LensList lenses() const;

    /**
     * @brief size
     * @return The number of entries in the Exif database
     */
    int size() const;

    /**
     * @brief Discards the current exif database and recreates it from the given files.
     *
     * Exiv2 seems to accept both image and movie files without ill effects
     * (but does not actually return any usable metadata).
     *
     * Recreating the exif database can take a lot of time. To get a decent user experience in spite of that,
     * the method updates the given AbstractProgressIndicator and calls QCoreApplication::processEvents()
     * in regular intervals (if a QCoreApplication instance is available).
     *
     * To be on the safe side though, you should still filter out non-image files as long as there is no official support for movie files in exiv2.
     * @param allImageFiles a list of all image files
     * @param progressIndicator for quick usage from a GUI application, use a DB::ProgressIndicator<QProgressDialog>
     */
    void recreate(const DB::FileNameList &allImageFiles, DB::AbstractProgressIndicator &progressIndicator);
    bool startInsertTransaction();
    bool commitInsertTransaction();
    bool abortInsertTransaction();

private:
    class DatabasePrivate;
    DatabasePrivate *d_ptr;
    Q_DECLARE_PRIVATE(Database)
};
}

#endif /* EXIFDATABASE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
