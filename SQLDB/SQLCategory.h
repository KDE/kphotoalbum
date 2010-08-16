/*
  Copyright (C) 2005-2010 Jesper K. Pedersen <blackie@kde.org>
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

#ifndef SQLCATEGORY_H
#define SQLCATEGORY_H

#include "DB/Category.h"
#include "DB/ImageSearchInfo.h"
#include "DB/ImageInfo.h" // DB::MediaType

namespace SQLDB {
    class SQLCategory: public DB::Category
    {
        Q_OBJECT

    public:
        virtual QMap<QString, uint> classify(const DB::ImageSearchInfo& scope,
                                             DB::MediaType typemask) const = 0;
    };
}

#endif /* SQLCATEGORY_H */
