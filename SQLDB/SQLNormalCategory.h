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

#ifndef SQLNORMALCATEGORY_H
#define SQLNORMALCATEGORY_H

#include "SQLTagCategory.h"

namespace SQLDB {
    class SQLNormalCategory: public SQLTagCategory
    {
        Q_OBJECT

    public:
        virtual void setName(const QString&);
        virtual void setSpecialCategory(bool) {}
        virtual bool isSpecialCategory() const { return false; }

    protected:
        friend class SQLCategoryCollection;
        SQLNormalCategory(QueryHelper* queryHelper, int categoryId):
            SQLTagCategory(queryHelper, categoryId) {}
    };
}

#endif /* SQLNORMALCATEGORY_H */
