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

#ifndef SQLDB_MYSQLCSG_H
#define SQLDB_MYSQLCSG_H

#include "CreateStatementGenerator.h"

namespace SQLDB
{
    namespace Schema
    {
        class MySQLCSG: public BaseCSG
        {
        public:
            virtual ~MySQLCSG() {}

        protected:
            virtual void
            makeTable(const TableSchema& t, list<string>& destList) const;

            virtual const char*
            getTypeKeyword(const Field& f) const;

            virtual const char*
            getConstraintKeyword(Constraint constraint) const;

            virtual void
            makeFieldType(const Field& f, string& destStr) const;
        };
    }
}

#endif /* SQLDB_MYSQLCSG_H */
