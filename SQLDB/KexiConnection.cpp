/*
  Copyright (C) 2006-2007 Tuomas Suutari <thsuut@utu.fi>

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

#include "KexiConnection.h"
#include "QueryErrors.h"
#include <memory>

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


KexiConnection::KexiConnection(KexiDB::Driver& driver,
                               const ConnectionParameters& connParams,
                               const QString& fileName):
    _cd(getKexiConnectionData(connParams))
{
    if (driver.isFileDriver()) {
        if (!fileName.isNull())
            _cd.setFileName(fileName);
        else
            _cd.setFileName(QString::fromLatin1("dummyDatabaseFile.db"));
    }

    std::auto_ptr<KexiDB::Connection> conn(driver.createConnection(_cd));

    if (!conn.get())
        throw ConnectionCreateError(driver.errorMsg());

    if (!conn->connect())
        throw ConnectionOpenError(conn->errorMsg());

    _driver = conn->driver();
    if (!_driver)
        throw InitializationError();

    // Don't use auto_ptr for connection, because KexiDB will delete
    // it when program is shutting down.
    _conn = conn.release();
}


// To print debug message for each executed SQL query
#define DEBUG_QUERYS


namespace
{
    KexiDB::Field::Type fieldTypeFor(QVariant::Type type)
    {
        switch (type) {
        case QVariant::Bool:
            return KexiDB::Field::Boolean;

        case QVariant::Int:
        case QVariant::UInt:
            return KexiDB::Field::Integer;

        case QVariant::LongLong:
        case QVariant::ULongLong:
            return KexiDB::Field::BigInteger;

        case QVariant::Double:
            return KexiDB::Field::Double;

        case QVariant::Date:
            return KexiDB::Field::Date;

        case QVariant::Time:
            return KexiDB::Field::Time;

        case QVariant::DateTime:
            return KexiDB::Field::DateTime;

        case QVariant::String:
        case QVariant::CString:
            return KexiDB::Field::Text;

        default:
            return KexiDB::Field::InvalidType;
        }
    }
}


QString KexiConnection::sqlRepresentation(const QVariant& x) const
{
    if (x.type() == QVariant::List) {
        QStringList repr;
        QValueList<QVariant> l = x.toList();
        for (QValueList<QVariant>::const_iterator i = l.begin();
             i != l.end(); ++i) {
            repr << sqlRepresentation(*i);
        }
        if (repr.isEmpty())
            return "NULL";
        else
            return repr.join(", ");
    }
    else
        // Escape and convert x to string
        return _driver->valueToSQL(fieldTypeFor(x.type()), x);
}

void KexiConnection::bindValues(QString &s, const Bindings& b) const
{
    int n = 0;
    for (Bindings::const_iterator i = b.begin(); i != b.end(); ++i) {
        n = s.find('?', n);
        if (n == -1)
            break;
        QString valAsSql(sqlRepresentation(*i));
        s = s.replace(n, 1, valAsSql);
        n += valAsSql.length();
    }
}

QueryResult KexiConnection::executeQuery(const QString& query,
                                         const Bindings& bindings) const
{
    QString q = query;
    bindValues(q, bindings);

#ifdef DEBUG_QUERYS
    qDebug("Executing query: %s", q.local8Bit().data());
#endif

#ifdef DEBUG_QUERY_TIMES
    QTime t;
    t.start();
#endif

    KexiDB::Cursor* c = _conn->executeQuery(q);
    if (!c) {
        throw QueryError(_conn->recentSQLString(),
                         _conn->errorMsg());
    }

#ifdef DEBUG_QUERY_TIMES
    int te = t.elapsed();
    if (te > 100) {
        {
            queryTimes.push_back(QPair<QString, uint>(q, te));
        }
    }
    qDebug("Time elapsed: %d ms", te);
#endif

    return QueryResult(c);
}

void KexiConnection::executeStatement(const QString& statement,
                                      const Bindings& bindings)
{
    QString s = statement;
    bindValues(s, bindings);

#ifdef DEBUG_QUERYS
    qDebug("Executing statement: %s", s.local8Bit().data());
#endif

    if (!_conn->executeSQL(s))
        throw StatementError(_conn->recentSQLString(),
                             _conn->errorMsg());
}

KexiConnection::RowId
KexiConnection::executeInsert(const QString& tableName,
                              const QString& aiFieldName,
                              const QStringList& fields,
                              const Bindings& values)
{
    Q_ASSERT(fields.count() == values.count());

    QString q = "INSERT INTO %1(%2) VALUES (%3)";
    q = q.arg(tableName);
    q = q.arg(fields.join(", "));
    QStringList l;
    for (Bindings::size_type i = values.count(); i > 0; --i)
        l.append("?");
    q = q.arg(l.join(", "));
    executeStatement(q, values);
    return _conn->lastInsertedAutoIncValue(aiFieldName, tableName);
}
