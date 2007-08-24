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

#ifndef SQLDB_DRIVERMANAGER_H
#define SQLDB_DRIVERMANAGER_H

#include "DatabaseManager.h"
#include "ConnectionParameters.h"
#include <kexidb/driver.h>
#include <kexidb/drivermanager.h>
#include <qstring.h>
#include <qstringlist.h>

namespace SQLDB
{
    class DriverInfo
    {
    public:
        const QString& name() const;
        bool isFileBased() const;

    private:
        friend class DriverManager;
        DriverInfo(const KexiDB::Driver::Info& kexiDriverInfo);

        KexiDB::Driver::Info _kexiDriverInfo;
    };

    class DriverManager
    {
    public:
        static DriverManager& instance();

        QStringList driverNames() const;

        DriverInfo getDriverInfo(const QString& driverName) const;

        DatabaseManager::APtr
        getDatabaseManager(const QString& driverName,
                           const ConnectionParameters& connParams=
                           ConnectionParameters()) const;

    private:
        static DriverManager* _instance;

        DriverManager();
        DriverManager(const DriverManager&);
        void operator=(const DriverManager&);

        mutable KexiDB::DriverManager _kexiDriverManager;
    };
}

#endif /* SQLDB_DRIVERMANAGER_H */
