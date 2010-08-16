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

#ifndef SQLDB_CONNECTION_H
#define SQLDB_CONNECTION_H

#include "QueryResult.h"
#include "Utilities/List.h"
#include <ksharedptr.h>
#include <QStringList>
#ifdef DEBUG_QUERY_TIMES
# include <QPair>
#endif

namespace SQLDB
{
    using Utilities::toVariantList;

    class Connection: public KShared
    {
    public:
        typedef QList<QVariant> Bindings;
        typedef quint64 RowId;

        virtual ~Connection()
        {
        }

        virtual QueryResult
        executeQuery(const char* query,
                     const Bindings& bindings=Bindings()) const = 0;

        virtual void
        executeStatement(const char* statement,
                         const Bindings& bindings=Bindings()) = 0;

        virtual RowId
        executeInsert(const QString& tableName,
                      const QString& aiFieldName,
                      const QStringList& fields,
                      const Bindings& values) = 0;

        virtual void beginTransaction() = 0;

        virtual void rollbackTransaction() = 0;

        virtual void commitTransaction() = 0;

#ifdef DEBUG_QUERY_TIMES
        mutable QList< QPair<QString, uint> > queryTimes;
#endif
    };

    typedef KSharedPtr<Connection> ConnectionSPtr;
}

#endif /* SQLDB_CONNECTION_H */
