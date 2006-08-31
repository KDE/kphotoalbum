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

#ifndef SQLDB_CURSOR_H
#define SQLDB_CURSOR_H

#include <kexidb/cursor.h>

namespace SQLDB
{
    using KexiDB::RowData;

    class Cursor
    {
    public:
        Cursor(KexiDB::Cursor* cursor): _cursor(cursor), _copies(0) {}

        Cursor(const Cursor& other):
            _cursor(other._cursor),
            _copies(other._copies)
        {
            if (!_copies) {
                _copies = new uint(1);
                other._copies = _copies;
            }
            ++*_copies;
        }

        Cursor& operator=(const Cursor& other)
        {
            if (&other != this) {
                this->~Cursor();
                _cursor = other._cursor;
                if (!other._copies)
                    _copies = other._copies = new uint(1);
                else
                    _copies = other._copies;
                ++*_copies;
            }
            return *this;
        }

        ~Cursor()
        {
            if (_copies) {
                --*_copies;
                if (*_copies == 0) {
                    delete _copies;
                    _copies = 0;
                }
            }
            if (_cursor && !_copies)
                _cursor->connection()->deleteCursor(_cursor);
        }

        bool isNull() const { return !_cursor; }
        operator bool() const { return _cursor; }

        // Do not call these if isNull()
        void selectFirstRow() { _cursor->moveFirst(); }
        bool selectNextRow() { return _cursor->moveNext(); }
        bool rowExists() const { return !_cursor->eof(); }
        void getCurrentRow(RowData& data) { _cursor->storeCurrentRow(data); }
        RowData getCurrentRow()
        {
            RowData data(_cursor->fieldCount());
            _cursor->storeCurrentRow(data);
            return data;
        }
        QVariant value(uint i) { return _cursor->value(i); }
        uint fieldCount() const { return _cursor->fieldCount(); }

    private:
        KexiDB::Cursor* _cursor;
        mutable uint* _copies;
    };
}

#endif /* SQLDB_CURSOR_H */
