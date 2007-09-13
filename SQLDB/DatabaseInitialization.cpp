/*
  Copyright (C) 2007 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "DatabaseInitialization.h"
#include "DriverManager.h"
#include "Schema/KPhotoAlbumSchema.h"
#include "QueryHelper.h"

// Update this when making incompatible schema change
#define SCHEMA_VERSION_MAJOR 2

// Update these every time the database schema changes
#define SCHEMA_VERSION_MINOR 1
#define SCHEMA_DATE "2006-11-01"

namespace
{
    static void insertInitialData(SQLDB::DatabaseConnection& conn)
    {
        struct
        {
            const char* name;
            const char* icon;
            bool visible;
            DB::Category::ViewType viewtype;
            int thumbnailSize;
        } entry[] = {
            { "Tokens", "bookmark",
              true, DB::Category::IconView, 32 },
            { "Keywords", "password",
              true, DB::Category::IconView, 32 },
            { "Places", "network",
              true, DB::Category::ListView, 32 },
            { "People", "user",
              true, DB::Category::ThumbedListView, 96 },
            { 0, 0, false, DB::Category::ListView, 0 }
        };

        SQLDB::QueryHelper qh(conn);

        for (int i = 0; entry[i].name != 0; ++i)
            qh.insertCategory(QString::fromLatin1(entry[i].name),
                              QString::fromLatin1(entry[i].icon),
                              entry[i].visible,
                              entry[i].viewtype, entry[i].thumbnailSize);
    }
}

SQLDB::DatabaseConnection
SQLDB::initializeKPhotoAlbumDatabase(const DatabaseAddress& address)
{
    DatabaseManager::APtr dbMgr
        (DriverManager::instance().
         getDatabaseManager(address.driverName(),
                            address.connectionParameters()));

    bool databaseCreated = false;

    const QString& dbName = address.databaseName();

    if (!dbMgr->databaseExists(dbName)) {
        dbMgr->createDatabase(dbName, Schema::getKPhotoAlbumSchema());
        databaseCreated = true;
    }

    DatabaseConnection dbConn(dbMgr->connectToDatabase(dbName));

    if (databaseCreated) {
        // Add schema version properties
        /*
        properties.setValue("schema version major", SCHEMA_VERSION_MAJOR);
        properties.setValue("schema version minor", SCHEMA_VERSION_MINOR);
        properties.setValue("schema date", SCHEMA_DATE);
        */

        insertInitialData(dbConn);
    }
    else {
        // Test schema version
        /*
        QVariant version =
            _connection->databaseProperties().value("schema version major");
        if (version.isNull())
            throw DatabaseSchemaError(i18n("Database schema is incompatible."));
        if (version.toUInt() != SCHEMA_VERSION_MAJOR)
            throw DatabaseSchemaError(i18n("Database version is incompatible."));
        */
    }

    return dbConn;
}
