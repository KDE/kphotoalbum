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

#ifndef QUERYRESULT_H
#define QUERYRESULT_H

#include "Cursor.h"
#include <qstringlist.h>
#include <qvariant.h>
#include <qpair.h>
#include <qmap.h>
//Added by qt3to4:
#include <Q3ValueList>

namespace SQLDB
{
    class QueryResult
    {
    public:
        QueryResult(KexiDB::Cursor* cursor): _cursor(cursor) {}

        QStringList asStringList() const;
        Q3ValueList<QString[2]> asString2List() const;
        Q3ValueList<QString[3]> asString3List() const;
        Q3ValueList<int> asIntegerList() const;
        Q3ValueList<uint> asUIntegerList() const;
        Q3ValueList< QPair<int, int> > asInteger2List() const;
        Q3ValueList< QPair<int, QString> > asIntegerStringPairs() const;
        QMap<int, QString> asIntegerStringMap() const;
        QMap<QString, uint> asStringUIntegerMap() const;
        Q3ValueList<QVariant> asVariantList() const;
        QVariant firstItem() const;
        RowData getRow(uint n=0) const;
        Cursor cursor() const { return _cursor; }

    private:
        mutable Cursor _cursor;
    };
}

#endif /* QUERYRESULT_H */
