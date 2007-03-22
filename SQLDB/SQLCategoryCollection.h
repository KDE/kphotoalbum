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
#ifndef SQLCATEGORYCOLLECTION_H
#define SQLCATEGORYCOLLECTION_H

#include "DB/CategoryCollection.h"
#include "Connection.h"
#include "QueryHelper.h"

namespace SQLDB {
    class SQLCategoryCollection :public DB::CategoryCollection
    {
        Q_OBJECT

    public:
        explicit SQLCategoryCollection(QueryHelper& queryHelper);

        virtual DB::CategoryPtr categoryForName(const QString& name) const;
        virtual QStringList categoryNames() const;
        virtual void removeCategory( const QString& name );
        virtual void rename( const QString& oldName, const QString& newName );
        virtual QValueList<DB::CategoryPtr> categories() const;
        virtual void addCategory(const QString& text, const QString& icon,
                                 DB::Category::ViewType type,
                                 int thumbnailSize, bool show );

    protected:
        QueryHelper& _qh;

    private:
        QStringList _specialCategoryNames;
    };
}

#endif /* SQLCATEGORYCOLLECTION_H */

