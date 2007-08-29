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

#ifndef SQLDB_DATABASEADDRESS_H
#define SQLDB_DATABASEADDRESS_H

#include "ConnectionParameters.h"
#include <qstring.h>

namespace SQLDB
{
    /** Stores connection parameters and database name.
     */
    class DatabaseAddress
    {
    public:
        DatabaseAddress():
            _driverName(),
            _fileBased(false),
            _db(),
            _connParams()
        {
        }

        bool operator==(const DatabaseAddress& other) const;

        bool isNull() const
        {
            return _driverName.isNull() || _db.isNull();
        }

        void setDriverName(const QString& driverName)
        {
            _driverName = driverName;
        }

        const QString& driverName() const
        {
            return _driverName;
        }

        void setFileBased(bool b)
        {
            _fileBased = b;
        }

        bool isFileBased() const
        {
            return _fileBased;
        }

        /** Sets name of the database.
         *
         * For file based address this is the file path.
         */
        void setDatabaseName(const QString& databaseName)
        {
            _db = databaseName;
        }

        const QString& databaseName() const
        {
            return _db;
        }

        const ConnectionParameters& connectionParameters() const
        {
            return _connParams;
        }

        void setToUseLocalConnection()
        {
            _connParams.setToLocal();
        }

        bool usesLocalConnection() const
        {
            return _connParams.isLocal();
        }

        void setHost(const QString& hostName, int port=0)
        {
            _connParams.setHostName(hostName);
            _connParams.setPort(port);
        }

        const QString& hostName() const
        {
            return _connParams.hostName();
        }

        int port() const
        {
            return _connParams.port();
        }

        void setUserName(const QString& userName)
        {
            _connParams.setUserName(userName);
        }

        const QString& userName() const
        {
            return _connParams.userName();
        }

        void setPassword(const QString& password)
        {
            _connParams.setPassword(password);
        }

        const QString& password() const
        {
            return _connParams.password();
        }

    private:
        QString _driverName;
        bool _fileBased;
        QString _db;
        ConnectionParameters _connParams;
    };
}

#endif /* SQLDB_DATABASEADDRESS_H */
