/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

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

#ifndef SQLDB_DATABASE_ADDRESS_H
#define SQLDB_DATABASE_ADDRESS_H

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
            _hostName(),
            _port(0),
            _userName(),
            _password()
        {
        }

        bool operator==(const DatabaseAddress& other) const;

        bool isNull() const
        {
            return _db.isNull();
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
         * For file based address this is the file name.
         */
        void setDatabaseName(const QString& databaseName)
        {
            _db = databaseName;
        }

        const QString& databaseName() const
        {
            return _db;
        }

        void setToUseLocalConnection()
        {
            setHost(QString());
        }

        bool usesLocalConnection() const
        {
            return hostName().isNull();
        }

        void setHost(const QString& hostName, int port=0)
        {
            if (!hostName.isEmpty())
                _hostName = hostName;
            _port = port;
        }

        const QString& hostName() const
        {
            return _hostName;
        }

        int port() const
        {
            return _port;
        }

        void setUserName(const QString& userName)
        {
            _userName = userName;
        }

        const QString& userName() const
        {
            return _userName;
        }

        void setPassword(const QString& password)
        {
            _password = password;
        }

        const QString& password() const
        {
            return _password;
        }

    private:
        QString _driverName;
        bool _fileBased;
        QString _db;
        QString _hostName;
        int _port;
        QString _userName;
        QString _password;
    };
}

#endif /* SQLDB_DATABASE_ADDRESS_H */
