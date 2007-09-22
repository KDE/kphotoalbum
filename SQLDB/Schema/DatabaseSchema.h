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

#ifndef SQLDB_DATABASESCHEMA_H
#define SQLDB_DATABASESCHEMA_H

#include <string>
#include <list>
#include <map>
#include <set>

namespace SQLDB
{
    namespace Schema
    {
        using std::string;
        using std::list;
        using std::map;
        using std::set;

        // ===============================================================

        template <class T>
        class AutoDeletingList: public list<T*>
        {
            typedef list<T*> B;
        public:
            AutoDeletingList() {}
            ~AutoDeletingList()
            {
                for (typename B::const_iterator i = B::begin();
                     i != B::end(); ++i)
                    delete *i;
            }

        private:
            AutoDeletingList(const AutoDeletingList&);
            void operator=(const AutoDeletingList&);
        };

        // ===============================================================

        class StringTuple: public list<string>
        {
        public:
            StringTuple() {}
            StringTuple(const list<string>& o): list<string>(o) {}
            StringTuple(const char* v0, const char* v1=0, const char* v2=0)
            {
                push_back(v0);
                if (!v1)
                    return;
                push_back(v1);
                if (!v2)
                    return;
                push_back(v2);
            }
        };

        typedef AutoDeletingList<StringTuple> StringTupleList;

        // ===============================================================

        enum DataType {
            TinyInteger,
            SmallInteger,
            Integer,
            BigInteger,
            UnsignedTinyInteger,
            UnsignedSmallInteger,
            UnsignedInteger,
            UnsignedBigInteger,
            Boolean,
            Decimal,
            Char,
            Varchar,
            Text,
            Timestamp,
            Date,
            Time
        };

        enum Constraint {
            NotNull,
            AutoIncrement,
            Unique
        };

        enum CharacterSet {
            DefaultCharacterSet,
            Ascii,
            Latin1,
            UTF8
        };

        enum Policy {
            NoAction,
            Restrict,
            Cascade,
            SetNull,
            SetDefault
        };

        // ===============================================================

        class FieldType
        {
        public:
            static const int noLength = -1;

            FieldType(DataType dataType,
                      int length1=noLength,
                      int length2=noLength);

            DataType dataType() const;
            int length1() const;
            int length2() const;

        private:
            DataType _dataType;
            int _length1;
            int _length2;
        };

        // ===============================================================

        class Field
        {
        public:
            Field(const string& name, FieldType type);

            void addConstraint(Constraint constraint);
            void setDefaultValue(const string& defaultValue);
            void setCharacterSet(CharacterSet characterSet);

            const string& name() const;
            const FieldType& type() const;
            const set<Constraint>& constraints() const;
            const string& defaultValue() const;
            CharacterSet characterSet() const;

        private:
            string _name;
            FieldType _type;
            set<Constraint> _constraints;
            string _defaultValue;
            CharacterSet _characterSet;
        };

        typedef AutoDeletingList<Field> FieldList;

        // ===============================================================

        class ForeignKey
        {
        public:
            ForeignKey(const StringTuple& sourceFields);

            void setDestination(const string& table,
                                const StringTuple& fields);
            void setDeletePolicy(Policy policy);
            void setUpdatePolicy(Policy policy);

            const StringTuple& sourceFields() const;
            const string& destinationTable() const;
            const StringTuple& destinationFields() const;
            Policy deletePolicy() const;
            Policy updatePolicy() const;

        private:
            StringTuple _srcFields;
            string _dstTable;
            StringTuple _dstFields;
            Policy _deletePolicy;
            Policy _updatePolicy;
        };

        typedef AutoDeletingList<ForeignKey> ForeignKeyList;

        // ===============================================================

        class TableSchema
        {
        public:
            TableSchema(const string& name);

            Field* createField(const string& name, const FieldType& type);
            void setPrimaryKey(const StringTuple& fields);
            ForeignKey* createForeignKey(const StringTuple& fields);
            void setUnique(const StringTuple& fields);
            void setIndexed(const StringTuple& fields);

            const string& name() const;
            const FieldList& fields() const;
            const StringTuple& primaryKey() const;
            const ForeignKeyList& foreignKeys() const;
            const StringTupleList& uniqueFieldTuples() const;
            const StringTupleList& indexedFieldTuples() const;

       private:
            bool hasField(const string& name) const;
            bool hasFields(const StringTuple& fields) const;

            string _name;
            FieldList _fields;
            map<string, Field*> _fieldMap;
            StringTuple _primaryKey;
            ForeignKeyList _foreignKeys;
            StringTupleList _uniqueFieldTuples;
            StringTupleList _indexedFieldTuples;
        };

        typedef AutoDeletingList<TableSchema> TableList;

        // ===============================================================

        class Identifier
        {
        public:
            /** Initialize new identifier with given name and version.
             *
             * \param name name of the identifier, may only have lower
             * case ASCII letters and '_' characters.
             */
            Identifier(const string& name, int versioMajor, int versionMinor);

            /** Set the last modification date of this identifier.
             *
             * Returns reference to this identifier.
             */
            Identifier& setDate(int year, int month, int day);

            /** Return true, iff this is compatible with the other.
             *
             * Objects with the same name and major version are
             * compatible.
             */
            bool isCompatibleWith(const Identifier& other) const;

            const string& name() const;
            int versionMajor() const;
            int versionMinor() const;
            int dateYear() const;
            int dateMonth() const;
            int dateDay() const;

        private:
            string _name;
            int _versionMajor;
            int _versionMinor;
            int _year;
            int _month;
            int _day;
        };

        // ===============================================================

        class DatabaseSchema
        {
        public:
            /** Initialize new database schema with given identifier.
             *
             * Increase the minor version of the identifier every time
             * you make a schema change that doesn't require any
             * change to the query statements of the application(s).
             */
            DatabaseSchema(const Identifier& identifier);

            /** Create a new table into this schema.
             *
             * Returns pointer to created TableSchema object. Use it
             * to set fields and other parameters of the table.
             *
             * When adding foreign keys to table make sure that the
             * referenced table is created before the referencing
             * table.
             */
            TableSchema* createTable(const string& name);

            /** Get list of the tables in this schema.
             *
             * The order of the returned tables is such that it is
             * possible to create them in that order. I.e. if creating
             * table X requires table Y to be created, Y is before X in
             * the list.
             */
            const TableList& tables() const;

            /** Get identifier of this schema.
             */
            const Identifier& identifier() const;

        private:
            bool hasTable(const string& name) const;

            Identifier _id;
            TableList _tables;
            map<string, TableSchema*> _tableMap;
        };
    }
}

#endif /* SQLDB_DATABASESCHEMA_H */
