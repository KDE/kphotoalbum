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

#include "DatabaseConnection.h"
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

    // Don't use auto_ptr for connection, because KexiDB will delete
    // it when program is shutting down.
    _conn = conn.release();
}
