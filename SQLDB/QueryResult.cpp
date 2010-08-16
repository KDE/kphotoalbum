/*
  Copyright (C) 2006-2010 Tuomas Suutari <thsuut@utu.fi>

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
#include "DB/RawId.h"
#include <QList>

using namespace SQLDB;

namespace
{
    template <class T>
    inline static T variantTo(const QVariant& variant)
    {
        return variant.value<T>();
    }

    template <>
    inline DB::RawId variantTo(const QVariant& variant)
    {
        return DB::RawId(variant.value<int>());
    }

    template <class T>
    inline QList<T> readCursor(SQLDB::Cursor& c)
    {
        QList<T> r;
        if (c)
            for (c.selectFirstRow(); c.rowExists(); c.selectNextRow())
                r.append(variantTo<T>(c.value(0)));
        return r;
    }

    template <class T1, class T2>
    inline QList< QPair<T1, T2> > readCursor2(SQLDB::Cursor& c)
    {
        QList< QPair<T1, T2> > r;
        if (c)
            for (c.selectFirstRow(); c.rowExists(); c.selectNextRow())
                r.append(QPair<T1, T2>(variantTo<T1>(c.value(0)),
                                       variantTo<T2>(c.value(1))));
        return r;
    }
}

template <class T>
QList<T> QueryResult::asList() const
{
    return readCursor<T>(_cursor);
}

namespace SQLDB {

template <>
QList< QPair<QString, QString> > QueryResult::asList() const
{
    return readCursor2<QString, QString>(_cursor);
}

}

// Explicit instantiations of QueryResult::asList
template QList<int> QueryResult::asList<int>() const;
#ifndef DB_RAWID_IS_PLAIN_INTEGER
template QList<DB::RawId> QueryResult::asList<DB::RawId>() const;
#endif
template QList<QString> QueryResult::asList<QString>() const;
#ifndef Q_CC_MSVC
template QList< QPair<QString, QString> > QueryResult::asList< QPair<QString, QString> >() const;
#endif

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
