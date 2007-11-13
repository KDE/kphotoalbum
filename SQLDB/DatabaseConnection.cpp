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

#include "DatabaseConnection.h"
#include "QueryErrors.h"
#include <QList>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlError>
#include <memory>

using namespace SQLDB;

// To print debug message for each executed SQL query
#define DEBUG_QUERYS

DatabaseConnection::DatabaseConnection(const QSqlDatabase& database):
    _database(database),
    _driver(_database.driver())
{
    if (!_driver)
        throw InitializationError();
}

QString DatabaseConnection::variantListAsSql(const QList<QVariant>& l) const
{
    QStringList repr;
    QSqlField f;
    for (QList<QVariant>::const_iterator i = l.begin();
         i != l.end(); ++i) {
        f.setType(i->type());
        f.setValue(*i);
        repr << _driver->formatValue(f);
    }
    if (repr.isEmpty())
        return QLatin1String("NULL");
    else
        return repr.join(QLatin1String(","));
}

void DatabaseConnection::processListParameters(QString& query,
                                               Bindings& bindings) const
{
    int mark = 0;
    Bindings::iterator i = bindings.begin();
    while (i != bindings.end()) {
        mark = query.indexOf('?', mark);
        if (mark == -1)
            break;

        if (i->type() == QVariant::List) {
            QString sql = variantListAsSql(i->toList());
            query.replace(mark, 1, sql);
            i = bindings.erase(i);
            mark += sql.length();
        }
        else {
            ++i;
            ++mark;
        }
    }
}

void DatabaseConnection::bindValues(QSqlQuery& query, const Bindings& b) const
{
    int n = 0;
    for (Bindings::const_iterator i = b.begin(); i != b.end(); ++i) {
        query.bindValue(n, *i);
        ++n;
    }
}

std::auto_ptr<QSqlQuery>
DatabaseConnection::initializeQuery(const QString& statement,
                                    const Bindings& bindings) const
{
    QString queryStr(statement);
    Bindings b(bindings);

    processListParameters(queryStr, b);

    std::auto_ptr<QSqlQuery> query(new QSqlQuery(queryStr, _database));
    bindValues(*query, b);

#ifdef DEBUG_QUERYS
    int lastPos = 0;
    for (Bindings::const_iterator i = b.begin(); i != b.end(); ++i) {
        lastPos = queryStr.indexOf('?', lastPos);
        if (lastPos == -1)
            break;
        QString x = i->toString();
        if (i->type() == QVariant::String ||
            i->type() == QVariant::DateTime ||
            i->type() == QVariant::Date ||
            i->type() == QVariant::Time)
            x = '\'' + x.replace('\'', "''") + '\'';
        queryStr.replace(lastPos, 1, x);
        lastPos += x.length();
    }
    qDebug("Initialized SQL: %s", queryStr.toLocal8Bit().constData());
#endif

    return query;
}

QueryResult DatabaseConnection::executeQuery(const QString& query,
                                             const Bindings& bindings) const
{
    std::auto_ptr<QSqlQuery> q(initializeQuery(query, bindings));

#ifdef DEBUG_QUERY_TIMES
    QTime t;
    t.start();
#endif

    if (!q->exec())
        throw QtSQLError(q->lastError());

#ifdef DEBUG_QUERY_TIMES
    int te = t.elapsed();
    if (te > 100) {
        {
            queryTimes.push_back(QPair<QString, uint>(q->executedQuery(), te));
        }
    }
    qDebug("Time elapsed: %d ms", te);
#endif

    return QueryResult(q);
}

void DatabaseConnection::executeStatement(const QString& statement,
                                          const Bindings& bindings)
{
    std::auto_ptr<QSqlQuery> s(initializeQuery(statement, bindings));

    if (!s->exec())
        throw QtSQLError(s->lastError());
}

qulonglong DatabaseConnection::executeInsert(const QString& tableName,
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


    std::auto_ptr<QSqlQuery> s(initializeQuery(q, values));

    if (!s->exec())
        throw QtSQLError(s->lastError());

    Q_UNUSED(aiFieldName);

    return s->lastInsertId().toULongLong();
}
