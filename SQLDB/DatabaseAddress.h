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

#ifndef SQLDB_DATABASE_ADDRESS_H
#define SQLDB_DATABASE_ADDRESS_H

#include <kexidb/kexidb_export.h>
#include <kexidb/connectiondata.h>

namespace SQLDB
{
    /** Stores connection parameters and database name.
     */
    class DatabaseAddress
    {
    public:
        DatabaseAddress() {}
        DatabaseAddress(const KexiDB::ConnectionData& connectionData,
                        const QString& databaseName):
            _cd(connectionData), _db(databaseName) {}
        bool operator==(const DatabaseAddress& other) const;
        const KexiDB::ConnectionData& connectionData() const { return _cd; }
        const QString& databaseName() const { return _db; }

    private:
        KexiDB::ConnectionData _cd;
        QString _db;
    };
}

#endif /* SQLDB_DATABASE_ADDRESS_H */
