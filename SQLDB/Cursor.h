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

#include <kexidb/cursor.h>
#include <qmap.h>

namespace SQLDB
{
    using KexiDB::RowData;

    class Cursor
    {
    public:
        Cursor(KexiDB::Cursor* cursor): _cursor(cursor)
        {
            if (!_cursor)
                throw Error(/* TODO: type and message */);
            ++_references[_cursor];
        }

        Cursor(const Cursor& other):
            _cursor(other._cursor)
        {
            ++_references[_cursor];
        }

        Cursor& operator=(const Cursor& other)
        {
            if (--_references[_cursor] == 0)
                _cursor->connection()->deleteCursor(_cursor);
            _cursor = other._cursor;
            ++_references[_cursor];
            return *this;
        }

        ~Cursor()
        {
            if (--_references[_cursor] == 0)
                _cursor->connection()->deleteCursor(_cursor);
        }

        inline
        void selectFirstRow() { _cursor->moveFirst(); }

        inline
        void selectNextRow() { _cursor->moveNext(); }

        inline
        bool rowExists() const { return !_cursor->eof(); }

        inline
        void getCurrentRow(RowData& data) { _cursor->storeCurrentRow(data); }

        inline
        RowData getCurrentRow()
        {
            RowData data;
            _cursor->storeCurrentRow(data);
            return data;
        }

    private:
        KexiDB::Cursor* _cursor;
        static QMap<KexiDB::Cursor*, uint> _references;
    };
}
