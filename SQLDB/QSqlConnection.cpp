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

#include "QSqlConnection.h"
#include "QueryErrors.h"
#include "Utilities/QStr.h"
#include <QList>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlError>
#include <memory>

using namespace SQLDB;

// To print debug message for each executed SQL query
//#define DEBUG_QUERYS

QSqlConnection::QSqlConnection(const QSqlDatabase& database):
    _database(database),
    _driver(_database.driver())
{
    if (!_driver)
        throw InitializationError();

    // TODO: There should be some kind of a check for this in
    // somewhere else and notification for the user too
    Q_ASSERT(_driver->hasFeature(QSqlDriver::LastInsertId));
}

QString QSqlConnection::variantListAsSql(const QList<QVariant>& l) const
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

void QSqlConnection::processListParameters(QString& query,
                                               Bindings& bindings) const
{
    int mark = 0;
    Bindings::iterator i = bindings.begin();
    while (i != bindings.end()) {
        mark = query.indexOf(QChar::fromLatin1('?'), mark);
        if (mark == -1)
            break;

        QList<QVariant> iAsList(i->toList());
        if (!iAsList.isEmpty()) {
            // val contained a list
            QString sql = variantListAsSql(iAsList);
            query.replace(mark, 1, sql);
            i = bindings.erase(i);
            mark += sql.length();
        }
        else {
            // val did not contain a list (or it contained an empty
            // list, which should never be passed as a binding)
            ++i;
            ++mark;
        }
    }
}

void QSqlConnection::bindValues(QSqlQuery& query, const Bindings& b) const
{
    int n = 0;
    for (Bindings::const_iterator i = b.begin(); i != b.end(); ++i) {
        query.bindValue(n, *i);
        ++n;
    }
}

std::auto_ptr<QSqlQuery>
QSqlConnection::initializeQuery(const QString& statement,
                                    const Bindings& bindings) const
{
    QString queryStr(statement);
    Bindings b(bindings);

    processListParameters(queryStr, b);

    std::auto_ptr<QSqlQuery> query(new QSqlQuery(_database));
    if (!query->prepare(queryStr))
        throw QtSQLError(*query, QLatin1String("Could not prepare a query"));
    bindValues(*query, b);

#ifdef DEBUG_QUERYS
    int lastPos = 0;
    for (Bindings::const_iterator i = b.begin(); i != b.end(); ++i) {
        lastPos = queryStr.indexOf(QChar::fromLatin1('?'), lastPos);
        if (lastPos == -1)
            break;
        QString x = i->toString();
        if (i->type() == QVariant::String ||
            i->type() == QVariant::DateTime ||
            i->type() == QVariant::Date ||
            i->type() == QVariant::Time)
            x = QChar::fromLatin1('\'') + x.replace(QChar::fromLatin1('\''), QString::fromLatin1("''")) + QChar::fromLatin1('\'');
        queryStr.replace(lastPos, 1, x);
        lastPos += x.length();
    }
    qDebug("Initialized SQL: %s", queryStr.toLocal8Bit().constData());
#endif

    return query;
}

QueryResult QSqlConnection::executeQuery(const char* query,
                                             const Bindings& bindings) const
{
    std::auto_ptr<QSqlQuery> q(
        initializeQuery(QString::fromUtf8(query), bindings));

#ifdef DEBUG_QUERY_TIMES
    QTime t;
    t.start();
#endif

    if (!q->exec())
        throw QtSQLError(*q, QLatin1String("Error while excuting a query"));

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

void QSqlConnection::executeStatement(const char* statement,
                                          const Bindings& bindings)
{
    std::auto_ptr<QSqlQuery> s(
        initializeQuery(QString::fromUtf8(statement), bindings));

    if (!s->exec())
        throw QtSQLError(*s, QLatin1String("Error while excuting a statement"));
}

QSqlConnection::RowId
QSqlConnection::executeInsert(const QString& tableName,
                              const QString& aiFieldName,
                              const QStringList& fields,
                              const Bindings& values)
{
    Q_ASSERT(fields.count() == values.count());

    QString q = QString::fromLatin1("INSERT INTO %1(%2) VALUES (%3)");
    q = q.arg(tableName);
    q = q.arg(fields.join(QString::fromLatin1(", ")));
    QStringList l;
    for (Bindings::size_type i = values.count(); i > 0; --i)
        l.append(QString::fromLatin1("?"));
    q = q.arg(l.join(QString::fromLatin1(", ")));


    std::auto_ptr<QSqlQuery> s(initializeQuery(q, values));

    if (!s->exec())
        throw QtSQLError(*s, QLatin1String("Error while excuting an insert statement"));

    Q_UNUSED(aiFieldName);

    const QVariant idVariant = s->lastInsertId();
    Q_ASSERT(!idVariant.isNull());
    return idVariant.toULongLong();
}
