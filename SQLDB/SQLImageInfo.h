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
#ifndef SQLIMAGEINFO_H
#define SQLIMAGEINFO_H

#include "DB/ImageInfo.h"
#include "QueryHelper.h"

// TODO(Tuomas): This should just use the base DB::ImageInfo, no special thing
// for SQLDB. The base class should contain the fileid. And this one not the
// QueryHelper.
namespace SQLDB {
    class SQLImageInfo :public DB::ImageInfo
    {
    protected:
        friend class QueryHelper;
        SQLImageInfo(QueryHelper* queryHelper, DB::RawId rawId);

        // used by the QueryHelper.. will be removed when ImageInfo is
        // refactored
        void markAsNotNullAndNotDirty()
        {
            setIsNull(false);
            setIsDirty(false);
        }

        void load();
        void saveChanges();

        QueryHelper* _qh;

    private:
        DB::RawId _fileId;
    };
}

#endif /* SQLIMAGEINFO_H */
