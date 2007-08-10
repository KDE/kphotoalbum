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

#ifndef SQLDB_SQLITECSG_H
#define SQLDB_SQLITECSG_H

#include "CreateStatementGenerator.h"

namespace SQLDB
{
    namespace Schema
    {
        class SQLiteCSG: public BaseCSG
        {
        public:
            virtual ~SQLiteCSG() {}

        protected:
            virtual void
            makeTable(const TableSchema& t, list<string>& destList) const;

            virtual const char*
            getTypeKeyword(const Field& f) const;

            virtual void
            makeForeignKey(const ForeignKey& fk, string& destStr) const;

            virtual void
            makeForeignKeys(const TableSchema& t,
                            list<string>& destList) const;

            virtual void
            makeIdTest(const StringTuple& l1, const string& p2,
                       const StringTuple& l2, string& destStr) const;
        };
    }
}

#endif /* SQLDB_SQLITECSG_H */
