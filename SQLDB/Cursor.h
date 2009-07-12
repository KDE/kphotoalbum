/*
  Copyright (C) 2006-2007 Tuomas Suutari <thsuut@utu.fi>

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

#ifndef SQLDB_CURSOR_H
#define SQLDB_CURSOR_H

#include <QSqlQuery>
#include <QVector>
#include <QVariant>
#include <QSqlRecord>

#include <memory>

namespace SQLDB
{
    typedef QVector<QVariant> RowData;

    class Cursor
    {
    public:
        Cursor(std::auto_ptr<QSqlQuery> query):
            _q(query)
        {
            Q_ASSERT(_q.get());
        }

        Cursor(const Cursor& other):
            _q(new QSqlQuery(*(other._q)))
        {
        }

        Cursor& operator=(const Cursor& other)
        {
            _q.reset(new QSqlQuery(*(other._q)));
            return *this;
        }

        bool isNull() const
        {
            return !(_q->isActive() && _q->isSelect());
        }

        operator bool() const
        {
            return !isNull();
        }

        // Do not call these if isNull()
        void selectFirstRow()
        {
            _q->first();
        }

        bool selectNextRow()
        {
            return _q->next();
        }

        bool rowExists() const
        {
            return _q->isValid();
        }

        RowData getCurrentRow()
        {
            int fields = fieldCount();
            RowData data(fields);
            for (int i = 0; i < fields; ++i) {
                data[i] = _q->value(i);
            }
            return data;
        }

        QVariant value(int i)
        {
            return _q->value(i);
        }

        int fieldCount() const
        {
            return _q->record().count();
        }

    private:
        std::auto_ptr<QSqlQuery> _q;
    };
}

#endif /* SQLDB_CURSOR_H */
