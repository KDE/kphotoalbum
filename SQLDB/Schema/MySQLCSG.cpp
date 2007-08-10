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

#include "MySQLCSG.h"

using namespace SQLDB::Schema;

void MySQLCSG::makeTable(const TableSchema& t, list<string>& destList) const
{
    BaseCSG::makeTable(t, destList);
    destList.back() += " ENGINE=InnoDB";
}

const char* MySQLCSG::getTypeKeyword(const Field& f) const
{
    switch (f.type().dataType()) {
    case UnsignedTinyInteger:
        return "TINYINT UNSIGNED";
    case UnsignedSmallInteger:
        return "SMALLINT UNSIGNED";
    case UnsignedInteger:
        return "INTEGER UNSIGNED";
    case UnsignedBigInteger:
        return "BIGINT UNSIGNED";
    case Timestamp:
        return "DATETIME";
    default:
        return BaseCSG::getTypeKeyword(f);
    };
}

const char* MySQLCSG::getConstraintKeyword(Constraint c) const
{
    switch (c) {
    case AutoIncrement:
        return "AUTO_INCREMENT";
    default:
        return BaseCSG::getConstraintKeyword(c);
    }
}

void MySQLCSG::makeFieldType(const Field& f, string& destStr) const
{
    BaseCSG::makeFieldType(f, destStr);
    switch (f.characterSet()) {
    case Ascii:
        destStr += " CHARACTER SET ascii";
        break;
    case Latin1:
        destStr += " CHARACTER SET latin1";
        break;
    case UTF8:
        destStr += " CHARACTER SET utf8";
        break;
    default:
        break;
    }
}
