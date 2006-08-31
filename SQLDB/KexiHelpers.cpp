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

#include "KexiHelpers.h"

QStringList SQLDB::readStringsFromCursor(Cursor& cursor, int col)
{
    QStringList l;
    for (cursor.selectFirstRow(); cursor.rowExists(); cursor.selectNextRow())
        l.append(cursor.value(col).toString());
    return l;
}

namespace
{
template <size_t N>
inline QValueList<QString[N]> readStringNsFromCursor(SQLDB::Cursor& cursor)
{
    QValueList<QString[N]> l;
    QString v[N];
    for (cursor.selectFirstRow(); cursor.rowExists(); cursor.selectNextRow()) {
        for (size_t i = 0; i < N; ++i)
            v[i] = cursor.value(i).toString();
        l.append(v);
    }
    return l;
}
}

QValueList<QString[2]> SQLDB::readString2sFromCursor(Cursor& cursor)
{
    return readStringNsFromCursor<2>(cursor);
}

QValueList<QString[3]> SQLDB::readString3sFromCursor(Cursor& cursor)
{
    return readStringNsFromCursor<3>(cursor);
}

QValueList<int> SQLDB::readIntsFromCursor(Cursor& cursor, int col)
{
    QValueList<int> l;
    for (cursor.selectFirstRow(); cursor.rowExists(); cursor.selectNextRow())
        l.append(cursor.value(col).toInt());
    return l;
}

KexiDB::Field::Type SQLDB::fieldTypeFor(QVariant::Type type)
{
    switch (type) {
    case QVariant::Bool:
        return KexiDB::Field::Boolean;

    case QVariant::Int:
    case QVariant::UInt:
        return KexiDB::Field::Integer;

    case QVariant::LongLong:
    case QVariant::ULongLong:
        return KexiDB::Field::BigInteger;

    case QVariant::Double:
        return KexiDB::Field::Double;

    case QVariant::Date:
        return KexiDB::Field::Date;

    case QVariant::Time:
        return KexiDB::Field::Time;

    case QVariant::DateTime:
        return KexiDB::Field::DateTime;

    case QVariant::String:
    case QVariant::CString:
        return KexiDB::Field::Text;

    default:
        return KexiDB::Field::InvalidType;
    }
}
