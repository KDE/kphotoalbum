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

#ifndef SQLMD5MAP_H
#define SQLMD5MAP_H

#include "DB/MD5Map.h"
#include "QueryHelper.h"

namespace SQLDB
{
    class SQLMD5Map : public DB::MD5Map
    {
    public:
        explicit SQLMD5Map(QueryHelper& queryHelper);

        void insert(const QString& md5sum, const QString& fileName);
        QString lookup(const QString& md5sum);
        bool contains(const QString& md5sum);
        void clear();

    protected:
        QueryHelper& _qh;
    };
}

#endif /* SQLMD5MAP_H */
