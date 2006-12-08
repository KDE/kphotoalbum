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
#include "SQLImageInfo.h"
#include "QueryHelper.h"

using namespace SQLDB;

SQLDB::SQLImageInfo::SQLImageInfo(QueryHelper* queryHelper, int fileId):
    DB::ImageInfo(),
    _qh(queryHelper),
    _fileId(fileId)
{
    load();
}

void SQLDB::SQLImageInfo::load()
{
    _qh->getMediaItem(_fileId, *this);
    setIsNull(false);
    setIsDirty(false);
    delaySavingChanges(false); // Doesn't save because isDirty() == false
}

void SQLDB::SQLImageInfo::saveChanges()
{
    if (!isNull() && isDirty()) {
        _qh->updateMediaItem(_fileId, *this);
        setIsDirty(false);
    }
}
