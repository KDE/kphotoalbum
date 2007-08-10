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

#include "SQLiteCSG.h"

using namespace SQLDB::Schema;

void SQLiteCSG::makeTable(const TableSchema& t, list<string>& destList) const
{
    BaseCSG::makeTable(t, destList);
    makeForeignKeys(t, destList);
}

const char* SQLiteCSG::getTypeKeyword(const Field& f) const
{
    switch (f.type().dataType()) {
    case TinyInteger:
    case SmallInteger:
    case Integer:
    case BigInteger:
    case UnsignedTinyInteger:
    case UnsignedSmallInteger:
    case UnsignedInteger:
    case UnsignedBigInteger:
        return "INTEGER";
    default:
        break;
    }

    return BaseCSG::getTypeKeyword(f);
}

void SQLiteCSG::makeForeignKey(const ForeignKey&, string&) const
{
}

void
SQLiteCSG::makeForeignKeys(const TableSchema& t, list<string>& destList) const
{
    string destStr;
    int n = 0;
    const ForeignKeyList& fks = t.foreignKeys();
    for (ForeignKeyList::const_iterator i = fks.begin(); i != fks.end(); ++i) {
        const ForeignKey& fk = **i;
        destStr.clear();

        switch (fk.deletePolicy()) {
        case Cascade:
            destStr += "CREATE TRIGGER cascade_del_to_" + t.name();
            makeNumber(++n, destStr);
            destStr += " DELETE ON " + fk.destinationTable();
            destStr += " BEGIN ";
            destStr += "DELETE FROM " + t.name() + " WHERE ";
            makeIdTest(fk.sourceFields(), "OLD", fk.destinationFields(),
                       destStr);
            destStr += "; ";
            destStr += "END";

            break;

        default:
            // Not supported
            break;
        }

        if (!destStr.empty())
            destList.push_back(destStr);
    }
}

void SQLiteCSG::makeIdTest(const StringTuple& l1, const string& p2,
                           const StringTuple& l2, string& destStr) const
{
    StringTuple::const_iterator i1 = l1.begin();
    StringTuple::const_iterator i2 = l2.begin();
    while (i1 != l1.end() && i2 != l2.end()) {
        if (i1 != l1.begin())
            destStr += " AND ";
        destStr += *i1 + "=" + p2 + "." + *i2;
        ++i1;
        ++i2;
    }
}
