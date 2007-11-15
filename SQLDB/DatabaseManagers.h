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

#ifndef SQLDB_DATABASEMANAGERS_H
#define SQLDB_DATABASEMANAGERS_H

#include "DatabaseManager.h"
#include "ConnectionParameters.h"
#include "KexiConnection.h"

namespace SQLDB
{
    class KexiDBDatabaseManager: public DatabaseManager
    {
    public:
        KexiDBDatabaseManager(const ConnectionParameters& connParams,
                              KexiDB::Driver& driver);

        virtual QStringList databases() const;

        virtual bool databaseExists(const QString& databaseName) const;

        virtual void createDatabase(const QString& databaseName,
                                    const Schema::DatabaseSchema& schema);

        virtual ConnectionSPtr
        connectToDatabase(const QString& databaseName);

    protected:
        ConnectionParameters _connParams;
        KexiDB::Driver& _driver;
        mutable KexiConnectionSPtr _conn;

    private:
        KexiDBDatabaseManager(const KexiDBDatabaseManager&);
        void operator=(const KexiDBDatabaseManager&);
    };
}

#endif /* SQLDB_DATABASEMANAGERS_H */
