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

#ifndef SQLDB_CONNECTIONPARAMETERS_H
#define SQLDB_CONNECTIONPARAMETERS_H

#include <qstring.h>

namespace SQLDB
{
    /** Parameters needed to connect to database.
     */
    class ConnectionParameters
    {
    public:
        ConnectionParameters():
            _hostName(),
            _port(0),
            _userName(),
            _password()
        {
        }

        void setToLocal()
        {
            _hostName = QString();
        }

        void setHostName(const QString& hostName)
        {
            _hostName = hostName;
        }

        void setPort(int port)
        {
            _port = port;
        }

        void setUserName(const QString& userName)
        {
            _userName = userName;
        }

        void setPassword(const QString& password)
        {
            _password = password;
        }

        bool isLocal() const
        {
            return _hostName.isNull();
        }

        const QString& hostName() const
        {
            return _hostName;
        }

        int port() const
        {
            return _port;
        }

        const QString& userName() const
        {
            return _userName;
        }

        const QString& password() const
        {
            return _password;
        }

    private:
        QString _hostName;
        int _port;
        QString _userName;
        QString _password;
    };
}

#endif /* SQLDB_CONNECTIONPARAMETERS_H */
