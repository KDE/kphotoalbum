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
#include "QueryErrors.h"
#include "Settings/SettingsData.h"
#include <kexidb/kexidb_export.h> // Should be in connectiondata.h
#include <kexidb/connectiondata.h>
#include <kexidb/drivermanager.h>
#include <kconfig.h>
#include <qfileinfo.h>

#define DEFAULT_DRIVER QString::fromLatin1("SQLite")
#define DEFAULT_DATABASE QString::fromLatin1("kphotoalbum")

using namespace KexiDB;

void SQLDB::readConnectionParameters(const KConfig& config,
                                     KexiDB::ConnectionData& data,
                                     QString& databaseName)
{
    data.driverName =
        config.readEntry(QString::fromLatin1("dbms"), DEFAULT_DRIVER);
    DriverManager dm;
    Driver::Info driverInfo = dm.driverInfo(data.driverName);
    if (driverInfo.name.isEmpty())
        throw Error(dm.errorMsg());

    // Could be database name for network based DBMSs
    // or filename (without path) for file based DBMSs
    databaseName =
        config.readEntry(QString::fromLatin1("database"), DEFAULT_DATABASE);
    if (databaseName.isEmpty())
        databaseName = DEFAULT_DATABASE;

    if (driverInfo.fileBased) {
        // Remove path to restrict overwriting of files in other directories
        QFileInfo fi(databaseName);
        databaseName = fi.fileName();
        data.setFileName(Settings::SettingsData::instance()->imageDirectory() +
                         databaseName);
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
}
