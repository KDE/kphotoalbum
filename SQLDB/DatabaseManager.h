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

#ifndef SQLDB_DATABASEMANAGER_H
#define SQLDB_DATABASEMANAGER_H

#include "Schema/DatabaseSchema.h"
#include "Connection.h"
#include <qstring.h>
#include <qstringlist.h>
#include <memory>

namespace SQLDB
{
    class DatabaseManager
    {
    public:
        typedef std::auto_ptr<DatabaseManager> APtr;

        virtual ~DatabaseManager() {}

        virtual QStringList databases() const = 0;

        virtual bool databaseExists(const QString& databaseName) const
        {
            return databases().contains(databaseName);
        }

        virtual void createDatabase(const QString& databaseName,
                                    const Schema::DatabaseSchema& schema) = 0;

        virtual ConnectionSPtr
        connectToDatabase(const QString& databaseName) = 0;
    };
}

#endif /* SQLDB_DATABASEMANAGER_H */
