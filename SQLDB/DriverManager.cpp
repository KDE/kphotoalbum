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

#include "DriverManager.h"
#include "DatabaseManagers.h"
#include "QueryErrors.h"
#include <klocalizedstring.h>
#include <QSqlDatabase>

using namespace SQLDB;

DriverInfo::DriverInfo(const QString& name):
    _name(name)
{
}

const QString& DriverInfo::name() const
{
    return _name;
}

bool DriverInfo::isFileBased() const
{
    return (_name == QLatin1String("QSQLITE") ||
            _name == QLatin1String("QSQLITE2"));
}


DriverManager* DriverManager::_instance(0);

DriverManager& DriverManager::instance()
{
    if (!_instance)
        _instance = new DriverManager();
    return *_instance;
}

DriverManager::DriverManager()
{
}

QStringList DriverManager::driverNames() const
{
    return QSqlDatabase::drivers();
}

DriverInfo DriverManager::getDriverInfo(const QString& driverName) const
{
    QString driverNameUpper(driverName.toUpper());
    if (!driverNames().contains(driverNameUpper))
        throw DriverNotFoundError(i18n("Database driver not found: %1",
                                       driverNameUpper));
    return DriverInfo(driverNameUpper);
}

DatabaseManager::APtr
DriverManager::getDatabaseManager(const QString& driverName,
                                  const ConnectionParameters& connParams) const
{
    if (driverName == QLatin1String("QMYSQL"))
        return DatabaseManager::APtr
            (new MySQLDatabaseManager(connParams));
    if (driverName == QLatin1String("QPSQL"))
        return DatabaseManager::APtr
            (new PostgreSQLDatabaseManager(connParams));
    if (driverName == QLatin1String("QSQLITE"))
        return DatabaseManager::APtr
            (new SQLiteDatabaseManager());
    else
        throw DriverNotFoundError(i18n("Unknown or unsupported "
                                       "database driver: %1",
                                       driverName));
}
