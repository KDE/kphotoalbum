/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

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

#include "ConfigFileHandler.h"
#include "DatabaseAddress.h"
#include "DriverManager.h"
#include "QueryErrors.h"
#include "Settings/SettingsData.h"
#include <kconfig.h>
#include <klocale.h>
#include <qfileinfo.h>

#define DEFAULT_DRIVER QString::fromLatin1("SQLite3")
#define DEFAULT_DATABASE QString::fromLatin1("kphotoalbum")
#define DATABASE_FILE_EXTENSION QString::fromLatin1(".db")
#define DATABASE_FILE_ROOT Settings::SettingsData::instance()->imageDirectory()

SQLDB::DatabaseAddress SQLDB::readConnectionParameters(const KConfig& config)
{
    DatabaseAddress dbAddr;

    QString driver(config.readEntry(QString::fromLatin1("dbms"),
                                    DEFAULT_DRIVER));
    DriverInfo driverInfo = DriverManager::instance().getDriverInfo(driver);

    dbAddr.setDriverName(driver);
    dbAddr.setFileBased(driverInfo.isFileBased());

    // Could be database name for network based DBMSs or filename
    // (relative to image root or absolute) for file based DBMSs
    QString databaseName;
    if (config.hasKey(QString::fromLatin1("database")))
        databaseName = config.readEntry(QString::fromLatin1("database"));

    // Check if config file has empty database name or no database
    // name at all
    if (databaseName.isEmpty()) {
        databaseName = DEFAULT_DATABASE;
        if (dbAddr.isFileBased())
            databaseName += DATABASE_FILE_EXTENSION;
    }

    if (dbAddr.isFileBased()) {
        // Add image root if path is relative
        QFileInfo fi(databaseName);
        if (fi.isRelative())
            databaseName = DATABASE_FILE_ROOT + fi.filePath();
        else
            databaseName = fi.filePath();
    }
    else {
        if (config.hasKey(QString::fromLatin1("host"))) {
            int port = config.readUnsignedNumEntry(QString::fromLatin1("port"), 0);
            dbAddr.setHost
                (config.readEntry(QString::fromLatin1("host")), port);
        }
        if (config.hasKey(QString::fromLatin1("username")))
            dbAddr.setUserName
                (config.readEntry(QString::fromLatin1("username")));
        if (config.hasKey(QString::fromLatin1("password")))
            dbAddr.setPassword
                (config.readEntry(QString::fromLatin1("password")));
    }

    dbAddr.setDatabaseName(databaseName);

    return dbAddr;
}

void SQLDB::writeConnectionParameters(const DatabaseAddress& address,
                                      KConfig& config)
{
    const QString& databaseName = address.databaseName();
    config.writeEntry(QString::fromLatin1("dbms"), address.driverName());
    config.writeEntry(QString::fromLatin1("database"), databaseName);

    if (!address.usesLocalConnection()) {
        config.writeEntry(QString::fromLatin1("host"), address.hostName());
        if (address.port() != 0)
            config.writeEntry(QString::fromLatin1("port"), address.port());
    }

    if (!address.userName().isEmpty())
        config.writeEntry(QString::fromLatin1("username"), address.userName());
    if (!address.password().isEmpty())
        config.writeEntry(QString::fromLatin1("password"), address.password());
}
