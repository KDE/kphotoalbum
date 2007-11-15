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

#include "DatabaseManagers.h"
#include "TransactionGuard.h"
#include "QueryErrors.h"
#include "Schema/MySQLCSG.h"
#include "Schema/PostgreSQLCSG.h"
#include "Schema/SQLiteCSG.h"
#include <qfile.h>
#include <memory>

using namespace SQLDB;

namespace
{
    static SQLDB::Schema::CreateStatementGenerator::APtr
    getCSGByDriver(const KexiDB::Driver& driver)
    {
        using namespace SQLDB::Schema;
        QString driverName = QString::fromLatin1(driver.name()).upper();
        if (driverName == "SQLITE3")
            return CreateStatementGenerator::APtr(new SQLiteCSG());
        else if (driverName == "MYSQL")
            return CreateStatementGenerator::APtr(new MySQLCSG());
        else if (driverName == "POSTGRESQL")
            return CreateStatementGenerator::APtr(new PostgreSQLCSG());
        else
            throw DriverNotFoundError("Driver not supported");
    }
}

KexiDBDatabaseManager::
KexiDBDatabaseManager(const ConnectionParameters& connParams,
                      KexiDB::Driver& driver):
    _connParams(connParams),
    _driver(driver),
    _conn(new KexiConnection(_driver, _connParams))
{
}

QStringList KexiDBDatabaseManager::databases() const
{
    return _conn->kexi().databaseNames();
}

bool KexiDBDatabaseManager::databaseExists(const QString& databaseName) const
{
    if (_driver.isFileDriver())
        return QFile::exists(databaseName);
    else
        return _conn->kexi().databaseExists(databaseName);
}

void KexiDBDatabaseManager::
createDatabase(const QString& databaseName,
               const Schema::DatabaseSchema& schema)
{
    using namespace Schema;

    CreateStatementGenerator::APtr csg(getCSGByDriver(_driver));

    if (_driver.isFileDriver())
        _conn = new KexiConnection(_driver, _connParams, databaseName);

    if (!_conn->kexi().createDatabase(databaseName))
        throw DatabaseCreateError(_conn->kexi().errorMsg());
    if (!_conn->kexi().useDatabase(databaseName, false))
        throw DatabaseOpenError(_conn->kexi().errorMsg());

    TransactionGuard transaction(*_conn);

    const list<string>& x = csg->generateCreateStatements(schema);
    for (list<string>::const_iterator i = x.begin(); i != x.end(); ++i)
        if (!_conn->kexi().executeSQL(QString::fromUtf8(i->c_str(), i->length())))
            throw StatementError(_conn->kexi().recentSQLString(),
                                 _conn->kexi().errorMsg());

    transaction.commit();
}

ConnectionSPtr
KexiDBDatabaseManager::connectToDatabase(const QString& databaseName)
{
    std::auto_ptr<KexiConnection> conn
        (new KexiConnection(_driver, _connParams, databaseName));
    if (!conn->kexi().useDatabase(databaseName, false))
        throw DatabaseOpenError(conn->kexi().errorMsg());
    return ConnectionSPtr(conn.release());
}
