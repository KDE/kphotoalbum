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
#include <kexidb/connectiondata.h>

using namespace SQLDB;

namespace
{
    static KexiDB::ConnectionData
    getKexiConnectionData(const SQLDB::ConnectionParameters& connParams)
    {
        KexiDB::ConnectionData cd;

        if (connParams.isLocal())
            cd.hostName = QString();
        else {
            cd.hostName = connParams.hostName();
            cd.port = connParams.port();
        }
        cd.userName = connParams.userName();
        cd.password = connParams.password();

        return cd;
    }
}


KexiDBDatabaseManager::
KexiDBDatabaseManager(const ConnectionParameters& connParams,
                      const QString& driverName,
                      KexiDB::Driver* driver):
    _connParams(connParams),
    _driverName(driverName),
    _driver(driver),
    _conn(createConnection())
{
}

KexiDBDatabaseManager::~KexiDBDatabaseManager()
{
    delete _conn;
}

KexiDB::Connection* KexiDBDatabaseManager::createConnection()
{
    KexiDB::ConnectionData cd(getKexiConnectionData(_connParams));
    KexiDB::Connection* conn = _driver->createConnection(cd);

    if (!conn)
        throw ConnectionCreateError(_driver->errorMsg());

    if (!conn->connect()) {
        QString errorMsg(conn->errorMsg());
        delete conn;
        throw ConnectionOpenError(errorMsg);
    }

    return conn;
}

QStringList KexiDBDatabaseManager::databases() const
{
    return _conn->databaseNames();
}

bool KexiDBDatabaseManager::databaseExists(const QString& databaseName) const
{
    return _conn->databaseExists(databaseName);
}

void KexiDBDatabaseManager::
createDatabase(const QString& databaseName,
               const Schema::DatabaseSchema& schema)
{
    using namespace Schema;

    CreateStatementGenerator::APtr csg;
    QString driverName = _driverName.upper();
    if (driverName == "SQLITE3")
        csg.reset(new SQLiteCSG());
    else if (driverName == "MYSQL")
        csg.reset(new MySQLCSG());
    else if (driverName == "POSTGRESQL")
        csg.reset(new PostgreSQLCSG());
    else
        throw DriverNotFoundError("Driver not supported");

    if (!_conn->createDatabase(databaseName))
        throw DatabaseCreateError(_conn->errorMsg());
    if (!_conn->useDatabase(databaseName))
        throw DatabaseOpenError(_conn->errorMsg());

    TransactionGuard transaction(*_conn);

    const list<string>& x = csg->generateCreateStatements(schema);
    for (list<string>::const_iterator i = x.begin(); i != x.end(); ++i)
        if (!_conn->executeSQL(QString::fromUtf8(i->c_str(), i->length())))
            throw StatementError(_conn->recentSQLString(),
                                 _conn->errorMsg());

    transaction.commit();
}

DatabaseConnection
KexiDBDatabaseManager::connectToDatabase(const QString& databaseName)
{
    KexiDB::Connection* conn = createConnection();
    if (!conn->useDatabase(databaseName))
        throw DatabaseOpenError(conn->errorMsg());
    return conn;
}
