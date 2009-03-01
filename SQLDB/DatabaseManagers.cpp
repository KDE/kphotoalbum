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
#include "QSqlConnection.h"
#include "Schema/MySQLCSG.h"
#include "Schema/PostgreSQLCSG.h"
#include "Schema/SQLiteCSG.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFile>

using namespace SQLDB;

namespace
{
    static QString newDatabaseName(const QString& baseName)
    {
        // TODO: check that database name is not in use
        static int i(0);
        return baseName + QString::number(++i);
    }

    static void setConnectionParameters(QSqlDatabase& db,
                                        const ConnectionParameters& connParams)
    {
        if (!connParams.isLocal()) {
            db.setHostName(connParams.hostName());
            if (connParams.port())
                db.setPort(connParams.port());
        }
        if (!connParams.userName().isNull()) {
            db.setUserName(connParams.userName());
            if (!connParams.password().isNull())
                db.setPassword(connParams.password());
        }
    }
}


BaseDatabaseManager::
BaseDatabaseManager(const ConnectionParameters& connParams,
                    const QString& driverName,
                    const QString& someDbName,
                    const QString& selDbsQuery,
                    Schema::CreateStatementGenerator::APtr csg):
    _someDbName(someDbName),
    _selDbsQuery(selDbsQuery),
    _csg(csg),
    _connName(newDatabaseName(QString::fromLatin1("dbmgr"))),
    _db(QSqlDatabase::addDatabase(driverName, _connName))
{
    setConnectionParameters(_db, connParams);
    _db.setDatabaseName(_someDbName);
    if (!_db.open())
        throw QtSQLError(_db.lastError(), QLatin1String("Cannot open database"));
}

QStringList BaseDatabaseManager::databases() const
{
    QSqlQuery q(_db);

    if (!q.exec(_selDbsQuery))
        throw QtSQLError(q, QLatin1String("Error while listing databases"));

    QStringList dbList;
    while (q.next())
        dbList.push_back(q.value(0).toString());

    return dbList;
}

void BaseDatabaseManager::
createDatabase(const QString& databaseName,
               const Schema::DatabaseSchema& schema)
{
    using namespace Schema;

    QSqlQuery q(_db);

    if (!q.exec(QLatin1String("CREATE DATABASE ") + databaseName))
        throw QtSQLError(q, QLatin1String("Cannot create a database"));

    ConnectionSPtr newDb = connectToDatabase(databaseName);

    TransactionGuard transaction(*newDb);

    const list<string>& x = _csg->generateCreateStatements(schema);
    for (list<string>::const_iterator i = x.begin(); i != x.end(); ++i)
        newDb->executeStatement(i->c_str());

    transaction.commit();
}

ConnectionSPtr
BaseDatabaseManager::connectToDatabase(const QString& databaseName)
{
    QString newConnName(newDatabaseName(QString::fromLatin1("conn")));
    QSqlDatabase db = QSqlDatabase::cloneDatabase(_db, newConnName);
    db.setDatabaseName(databaseName);
    if (!db.open())
        throw QtSQLError(db.lastError(), QLatin1String("Cannot connect to database"));

    return ConnectionSPtr(new QSqlConnection(db));
}


MySQLDatabaseManager::
MySQLDatabaseManager(const ConnectionParameters& connParams):
    BaseDatabaseManager(connParams,
                        QLatin1String("QMYSQL"),
                        QLatin1String("INFORMATION_SCHEMA"),
                        QLatin1String("SHOW DATABASES"),
                        Schema::CreateStatementGenerator::APtr
                        (new Schema::MySQLCSG()))
{
}


PostgreSQLDatabaseManager::
PostgreSQLDatabaseManager(const ConnectionParameters& connParams):
    BaseDatabaseManager(connParams,
                        QLatin1String("QPSQL"),
                        QLatin1String("template1"),
                        QLatin1String("SELECT datname "
                                      "FROM pg_catalog.pg_database "
                                      "WHERE datallowconn "
                                      "AND NOT datistemplate"),
                        Schema::CreateStatementGenerator::APtr
                        (new Schema::PostgreSQLCSG()))
{
}


QStringList SQLiteDatabaseManager::databases() const
{
    return QStringList();
}

bool SQLiteDatabaseManager::databaseExists(const QString& databaseName) const
{
    return QFile(databaseName).exists();
}

void
SQLiteDatabaseManager::createDatabase(const QString& databaseName,
                                      const Schema::DatabaseSchema& schema)
{
    using namespace Schema;

    SQLiteCSG csg;

    ConnectionSPtr newDb = connectToDatabase(databaseName);

    TransactionGuard transaction(*newDb);

    const list<string>& x = csg.generateCreateStatements(schema);
    for (list<string>::const_iterator i = x.begin(); i != x.end(); ++i)
        newDb->executeStatement(i->c_str());

    transaction.commit();
}

ConnectionSPtr
SQLiteDatabaseManager::connectToDatabase(const QString& databaseName)
{
    QString newConnName(newDatabaseName(QString::fromLatin1("conn")));
    QSqlDatabase db = QSqlDatabase::addDatabase(QString::fromLatin1("QSQLITE"), newConnName);
    db.setDatabaseName(databaseName);

    if (!db.open())
        throw QtSQLError(db.lastError(), QLatin1String("Cannot open SQLite database"));

    return ConnectionSPtr(new QSqlConnection(db));
}
