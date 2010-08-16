/*
  Copyright (C) 2007-2010 Tuomas Suutari <thsuut@utu.fi>

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
#include "Schema/CreateStatementGenerator.h"

namespace SQLDB
{
    class BaseDatabaseManager: public DatabaseManager
    {
    public:
        BaseDatabaseManager(const ConnectionParameters& connParams,
                            const QString& driverName,
                            const QString& someDbName,
                            const QString& selDbsQuery,
                            Schema::CreateStatementGenerator::APtr csg);

        virtual QStringList databases() const;

        virtual void createDatabase(const QString& databaseName,
                                    const Schema::DatabaseSchema& schema);

        virtual ConnectionSPtr
        connectToDatabase(const QString& databaseName);

    private:
        BaseDatabaseManager(const BaseDatabaseManager&);
        void operator=(const BaseDatabaseManager&);

        QString _someDbName;
        QString _selDbsQuery;
        Schema::CreateStatementGenerator::APtr _csg;
        QString _connName;
        QSqlDatabase _db;
    };

    class MySQLDatabaseManager: public BaseDatabaseManager
    {
    public:
        MySQLDatabaseManager(const ConnectionParameters& connParams);
    };

    class PostgreSQLDatabaseManager: public BaseDatabaseManager
    {
    public:
        PostgreSQLDatabaseManager(const ConnectionParameters& connParams);
    };

    class SQLiteDatabaseManager: public DatabaseManager
    {
    public:
        virtual QStringList databases() const;

        virtual bool databaseExists(const QString& databaseName) const;

        virtual void createDatabase(const QString& databaseName,
                                    const Schema::DatabaseSchema& schema);

        virtual ConnectionSPtr
        connectToDatabase(const QString& databaseName);
    };
}

#endif /* SQLDB_DATABASEMANAGERS_H */
