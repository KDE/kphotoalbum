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

#ifndef SQLDB_CREATESTATEMENTGENERATOR_H
#define SQLDB_CREATESTATEMENTGENERATOR_H

#include "DatabaseSchema.h"
#include <memory>
#include <list>
#include <string>

namespace SQLDB
{
    namespace Schema
    {
        using std::list;
        using std::string;

        /** Generates SQL create statements from a database schema.
         *
         * With this interface it is possible to create SQL database
         * from a DatabaseSchema object in the ame way regardless of
         * the DBMS in use.
         */
        class CreateStatementGenerator
        {
        public:
            typedef std::auto_ptr<CreateStatementGenerator> APtr;

            virtual ~CreateStatementGenerator() {}

            /** Generate SQL commands for creating a database.
             *
             * Execute the returned commands to create a new database
             * with the given schema.
             */
            virtual const list<string>&
            generateCreateStatements(const DatabaseSchema& schema);

        protected:
            virtual void
            makeTable(const TableSchema& t, list<string>& destList) const = 0;

            virtual void
            makeIndices(const TableSchema& t,
                        list<string>& destList) const = 0;

            virtual void
            makeMetadataInserts(const Identifier& id,
                                list<string>& destList) const = 0;

        private:
            list<string> _statementList;
        };


        /** Basic CreateStatementGenerator implementation.
         *
         * Useful as a base class for other CSG classes. Just
         * overwrite the virtual methods you want to have different
         * behaviour.
         */
        class BaseCSG: public CreateStatementGenerator
        {
        public:
            virtual ~BaseCSG() {}

        protected:
            virtual const char*
            getTypeKeyword(const Field& f) const;

            virtual const char*
            getConstraintKeyword(Constraint c) const;

            virtual const char*
            getPolicyKeyword(Policy p) const;

            virtual void
            makeTable(const TableSchema& t, list<string>& destList) const;

            virtual void
            makeIndices(const TableSchema& t, list<string>& destList) const;

            virtual void
            makeMetadataInserts(const Identifier& id,
                                list<string>& destList) const;

            virtual void
            makeField(const Field& f, string& destStr) const;

            virtual void
            makeFieldSuffix(const Field& f, string& destStr) const;

            virtual void
            makeFieldType(const Field& f, string& destStr) const;

            virtual void
            makeFieldConstraints(const Field& f, string& destStr) const;

            virtual void
            makeFieldDefaultValue(const Field& f, string& destStr) const;

            virtual void
            makeConstraints(const TableSchema& t, string& destStr) const;

            virtual void
            makePrimaryKey(const StringTuple& pk, string& destStr) const;

            virtual void
            makeForeignKey(const ForeignKey& fk, string& destStr) const;

            virtual void
            makeUniqueIndices(const StringTupleList& ul,
                              string& destStr) const;

            virtual void
            makeStringTuple(const StringTuple& st, string& destStr) const;

            virtual void
            makeNumber(int n, string& destStr) const;
        };
    }
}

#endif /* SQLDB_CREATESTATEMENTGENERATOR_H */
