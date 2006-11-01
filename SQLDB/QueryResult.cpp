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

#include "QueryResult.h"
#include "QueryErrors.h"

using namespace SQLDB;

QStringList QueryResult::asStringList() const
{
    QStringList r;
    if (_cursor) {
        for (_cursor.selectFirstRow();
             _cursor.rowExists(); _cursor.selectNextRow())
            r.append(_cursor.value(0).toString());
    }
    return r;
}

namespace
{
    template <size_t N>
    inline QValueList<QString[N]> readStringNsFromCursor(SQLDB::Cursor& cursor)
    {
        QValueList<QString[N]> l;
        for (cursor.selectFirstRow();
             cursor.rowExists(); cursor.selectNextRow()) {
            QString v[N];
            for (size_t i = 0; i < N; ++i)
                v[i] = cursor.value(i).toString();
            l.append(v);
        }
        return l;
    }
}

QValueList<QString[2]> QueryResult::asString2List() const
{
    QValueList<QString[2]> r;
    if (_cursor) {
        r = readStringNsFromCursor<2>(_cursor);
    }
    return r;
}

QValueList<QString[3]> QueryResult::asString3List() const
{
    QValueList<QString[3]> r;
    if (_cursor) {
        r = readStringNsFromCursor<3>(_cursor);
    }
    return r;
}

QValueList<int> QueryResult::asIntegerList() const
{
    QValueList<int> r;
    if (_cursor) {
        for (_cursor.selectFirstRow();
             _cursor.rowExists(); _cursor.selectNextRow())
            r.append(_cursor.value(0).toInt());
    }
    return r;
}

QValueList< QPair<int, int> > QueryResult::asInteger2List() const
{
    QValueList< QPair<int, int> > r;
    if (_cursor) {
        for (_cursor.selectFirstRow();
             _cursor.rowExists(); _cursor.selectNextRow())
            r.append(QPair<int, int>(_cursor.value(0).toInt(),
                                     _cursor.value(1).toInt()));
    }
    return r;
}

QValueList< QPair<int, QString> > QueryResult::asIntegerStringPairs() const
{
    QValueList< QPair<int, QString> > r;
    if (_cursor) {
        for (_cursor.selectFirstRow(); _cursor.rowExists();
             _cursor.selectNextRow())
            r << QPair<int, QString>(_cursor.value(0).toInt(),
                                     _cursor.value(1).toString());
    }
    return r;
}

QMap<int, QString> QueryResult::asIntegerStringMap() const
{
    QMap<int, QString> r;
    if (_cursor) {
        for (_cursor.selectFirstRow(); _cursor.rowExists();
             _cursor.selectNextRow())
            r[_cursor.value(0).toInt()] = _cursor.value(1).toString();
    }
    return r;
}


QVariant QueryResult::firstItem() const
{
    QVariant r;
    if (_cursor) {
        _cursor.selectFirstRow();
        if (_cursor.rowExists())
             r = _cursor.value(0);
    }
    return r;
}

RowData QueryResult::getRow(uint n) const
{
    if (_cursor) {
        _cursor.selectFirstRow();
        for (uint i = 0; i < n; ++i) {
            if (!_cursor.selectNextRow())
                break;
        }
        if (_cursor.rowExists()) {
            return _cursor.getCurrentRow();
        }
    }
    throw RowNotFoundError();
}
