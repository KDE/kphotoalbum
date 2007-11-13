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

using namespace SQLDB;

DriverInfo::DriverInfo(const KexiDB::Driver::Info& kexiDriverInfo):
    _kexiDriverInfo(kexiDriverInfo)
{
}

const QString& DriverInfo::name() const
{
    return _kexiDriverInfo.name;
}

bool DriverInfo::isFileBased() const
{
    return _kexiDriverInfo.fileBased;
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
    return _kexiDriverManager.driverNames();
}

DriverInfo DriverManager::getDriverInfo(const QString& driverName) const
{
    KexiDB::Driver::Info driverInfo
        (_kexiDriverManager.driverInfo(driverName));

    if (driverInfo.name.isEmpty())
        throw DriverNotFoundError(_kexiDriverManager.errorMsg());

    return DriverInfo(driverInfo);
}

DatabaseManager::APtr
DriverManager::getDatabaseManager(const QString& driverName,
                                  const ConnectionParameters& connParams) const
{
    KexiDB::Driver* driver = _kexiDriverManager.driver(driverName);

    if (driver)
        return DatabaseManager::APtr
            (new KexiDBDatabaseManager(connParams, *driver));
    else
        throw DriverNotFoundError(_kexiDriverManager.errorMsg());
}
