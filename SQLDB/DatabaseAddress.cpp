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

#include "DatabaseAddress.h"
#include "Utilities/Util.h"
#include <kexidb/drivermanager.h>


bool SQLDB::DatabaseAddress::operator==(const DatabaseAddress& other) const
{
    if (_cd.driverName.lower() != other._cd.driverName.lower())
        return false;

    KexiDB::Driver::Info info =
        KexiDB::DriverManager().driverInfo(_cd.driverName);

    if (info.fileBased)
        return Utilities::areSameFile(_cd.fileName(), other._cd.fileName());

    if (_db != other._db)
        return false;

    if (_cd.useLocalSocketFile != other._cd.useLocalSocketFile)
        return false;

    if (_cd.useLocalSocketFile)
        return _cd.localSocketFileName == other._cd.localSocketFileName;

    return (_cd.hostName == other._cd.hostName &&
            _cd.port == other._cd.port);
}
