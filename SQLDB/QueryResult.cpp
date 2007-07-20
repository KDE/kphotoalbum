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

StringStringList QueryResult::asStringStringList() const
{
    StringStringList r;
    if (_cursor) {
        for (_cursor.selectFirstRow();
             _cursor.rowExists(); _cursor.selectNextRow())
            r.append(QPair<QString, QString>(_cursor.value(0).toString(),
                                             _cursor.value(1).toString()));
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
