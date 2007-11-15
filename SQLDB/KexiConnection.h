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

#ifndef SQLDB_KEXICONNECTION_H
#define SQLDB_KEXICONNECTION_H

#include "Connection.h"
#include "ConnectionParameters.h"
#include <kexidb/connection.h>
#include <kexidb/driver.h>

namespace SQLDB
{
    class KexiConnection: public Connection
    {
    public:
        KexiConnection(KexiDB::Driver& driver,
                       const ConnectionParameters& connParams,
                       const QString& fileName=QString::null);

        virtual QueryResult
        executeQuery(const QString& query,
                     const Bindings& bindings=Bindings()) const;

        virtual void
        executeStatement(const QString& statement,
                         const Bindings& bindings=Bindings());

        virtual RowId
        executeInsert(const QString& tableName,
                      const QString& aiFieldName,
                      const QStringList& fields,
                      const Bindings& values);

        virtual void beginTransaction()
        {
            KexiDB::Transaction t(kexi().beginTransaction());
            kexi().setDefaultTransaction(t);
        }

        virtual void rollbackTransaction()
        {
            kexi().rollbackTransaction();
        }

        virtual void commitTransaction()
        {
            kexi().commitTransaction();
        }

        KexiDB::Connection& kexi()
        {
            return *_conn;
        }

    protected:
        QString sqlRepresentation(const QVariant& x) const;
        void bindValues(QString &s, const Bindings& b) const;

    private:
        KexiDB::ConnectionData _cd;
        KexiDB::Connection* _conn;
        KexiDB::Driver* _driver;
    };

    typedef KSharedPtr<KexiConnection> KexiConnectionSPtr;
}

#endif /* SQLDB_KEXICONNECTION_H */
