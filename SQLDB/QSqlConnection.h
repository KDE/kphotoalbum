/*
  Copyright (C) 2006-2010 Tuomas Suutari <thsuut@utu.fi>

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

#ifndef SQLDB_QSQLCONNECTION_H
#define SQLDB_QSQLCONNECTION_H

#include "Connection.h"
#include "QueryResult.h"
#include "Utilities/List.h"
#include <QStringList>
#include <QList>
#include <QSqlDatabase>

namespace SQLDB
{
    class QSqlConnection: public Connection
    {
    public:
        explicit QSqlConnection(const QSqlDatabase& database);

        virtual QueryResult
        executeQuery(const char* query,
                     const Bindings& bindings=Bindings()) const;

        virtual void
        executeStatement(const char* statement,
                         const Bindings& bindings=Bindings());

        virtual RowId
        executeInsert(const QString& tableName,
                      const QString& aiFieldName,
                      const QStringList& fields,
                      const Bindings& values);

        virtual void beginTransaction()
        {
            _database.transaction();
        }

        virtual void rollbackTransaction()
        {
            _database.rollback();
        }

        virtual void commitTransaction()
        {
            _database.commit();
        }

    protected:
        QString variantListAsSql(const QList<QVariant>& l) const;

        void processListParameters(QString& query, Bindings& bindings) const;

        void bindValues(QSqlQuery& s, const Bindings& b) const;

        std::auto_ptr<QSqlQuery>
        initializeQuery(const QString& statement, const Bindings& bindings) const;

    private:
        QSqlDatabase _database;
        QSqlDriver* _driver;
    };
}

#endif /* SQLDB_QSQLCONNECTION_H */
