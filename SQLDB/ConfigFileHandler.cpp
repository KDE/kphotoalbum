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
#include "QueryErrors.h"
#include "Settings/SettingsData.h"
#include <kexidb/kexidb_export.h> // Should be in connectiondata.h
#include <kexidb/connectiondata.h>
#include <kexidb/drivermanager.h>
#include <kconfig.h>
#include <qfileinfo.h>

#define DEFAULT_DRIVER QString::fromLatin1("SQLite3")
#define DEFAULT_DATABASE QString::fromLatin1("kphotoalbum")
#define DATABASE_FILE_EXTENSION QString::fromLatin1(".db")
#define DATABASE_FILE_ROOT Settings::SettingsData::instance()->imageDirectory()

using namespace KexiDB;

SQLDB::DatabaseAddress SQLDB::readConnectionParameters(const KConfig& config)
{
    KexiDB::ConnectionData data;
    QString databaseName;

    data.driverName =
        config.readEntry(QString::fromLatin1("dbms"), DEFAULT_DRIVER);
    DriverManager dm;
    Driver::Info driverInfo = dm.driverInfo(data.driverName);
    if (driverInfo.name.isEmpty())
        throw DriverNotFoundError(dm.errorMsg());

    // Could be database name for network based DBMSs or filename
    // (relative to image root or absolute) for file based DBMSs
    databaseName = QString::null;
    if (config.hasKey(QString::fromLatin1("database")))
        databaseName = config.readEntry(QString::fromLatin1("database"));

    // Check if config file has empty database name or no database
    // name at all
    if (databaseName.isEmpty()) {
        databaseName = DEFAULT_DATABASE;
        if (driverInfo.fileBased)
            databaseName += DATABASE_FILE_EXTENSION;
    }

    if (driverInfo.fileBased) {
        // Add image root if path is relative
        QFileInfo fi(databaseName);
        if (fi.isRelative())
            databaseName = DATABASE_FILE_ROOT + fi.filePath();
        else
            databaseName = fi.filePath();
        data.setFileName(databaseName);
    }
    else {
        if (config.hasKey(QString::fromLatin1("host")))
            data.hostName = config.readEntry(QString::fromLatin1("host"));
        if (config.hasKey(QString::fromLatin1("port")))
            data.port =
                config.readUnsignedNumEntry(QString::fromLatin1("port"));
        if (config.hasKey(QString::fromLatin1("username")))
            data.userName = config.readEntry(QString::fromLatin1("username"));
        if (config.hasKey(QString::fromLatin1("password")))
            data.password = config.readEntry(QString::fromLatin1("password"));
    }

    return DatabaseAddress(data, databaseName);
}

void SQLDB::writeConnectionParameters(const DatabaseAddress& address,
                                      KConfig& config)
{
    const KexiDB::ConnectionData& data = address.connectionData();
    const QString& databaseName = address.databaseName();
    config.writeEntry(QString::fromLatin1("dbms"), data.driverName);
    config.writeEntry(QString::fromLatin1("database"), databaseName);

    if (!data.hostName.isEmpty())
        config.writeEntry(QString::fromLatin1("host"), data.hostName);
    if (data.port != 0)
        config.writeEntry(QString::fromLatin1("port"), data.port);

    if (!data.userName.isEmpty())
        config.writeEntry(QString::fromLatin1("username"), data.userName);
    if (!data.password.isEmpty())
        config.writeEntry(QString::fromLatin1("password"), data.password);
}
