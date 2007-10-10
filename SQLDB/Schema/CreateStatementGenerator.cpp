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

#include "CreateStatementGenerator.h"
#include <stdio.h> //snprintf

using namespace SQLDB::Schema;

const list<string>&
CreateStatementGenerator::
generateCreateStatements(const DatabaseSchema& schema)
{
    _statementList.clear();
    const TableList& tl = schema.tables();
    for (TableList::const_iterator t = tl.begin(); t != tl.end(); ++t) {
        makeTable(**t, _statementList);
        makeIndices(**t, _statementList);
    }
    makeMetadataInserts(schema.identifier(), _statementList);
    return _statementList;
}

const char* BaseCSG::getTypeKeyword(const Field& f) const
{
    switch (f.type().dataType()) {
    case TinyInteger:
        return "TINYINT";
    case SmallInteger:
        return "SMALLINT";
    case Integer:
        return "INTEGER";
    case BigInteger:
        return "BIGINT";
    case UnsignedTinyInteger:
        return "TINYINT";
    case UnsignedSmallInteger:
        return "SMALLINT";
    case UnsignedInteger:
        return "INTEGER";
    case UnsignedBigInteger:
        return "BIGINT";
    case Boolean:
        return "BOOLEAN";
    case Decimal:
        return "DECIMAL";
    case Char:
        return "CHAR";
    case Varchar:
        return "VARCHAR";
    case Text:
        return "TEXT";
    case Timestamp:
        return "TIMESTAMP";
    case Date:
        return "DATE";
    case Time:
        return "TIME";
    };
    return "";
}

const char* BaseCSG::getConstraintKeyword(Constraint c) const
{
    switch (c) {
    case NotNull:
        return "NOT NULL";
    case Unique:
        return "UNIQUE";
    case AutoIncrement:
        // Not in standard SQL
        return "";
    }
    return "";
}

const char* BaseCSG::getPolicyKeyword(Policy p) const
{
    switch (p) {
    case NoAction:
        return "NO ACTION";
    case Restrict:
        return "RESTRICT";
    case Cascade:
        return "CASCADE";
    case SetNull:
        return "SET NULL";
    case SetDefault:
        return "SET DEFAULT";
    };
    return "";
}

void BaseCSG::makeTable(const TableSchema& t, list<string>& destList) const
{
    string destStr = "CREATE TABLE " + t.name() + "(";
    const FieldList& fl = t.fields();
    for (FieldList::const_iterator f = fl.begin(); f != fl.end(); ++f) {
        if (f != fl.begin())
            destStr += ", ";
        makeField(**f, destStr);
    }
    makeConstraints(t, destStr);
    destStr += ")";
    destList.push_back(destStr);
}

void BaseCSG::makeField(const Field& f, string& destStr) const
{
    destStr += f.name() + " ";
    makeFieldSuffix(f, destStr);
}

void BaseCSG::makeFieldSuffix(const Field& f, string& destStr) const
{
    makeFieldType(f, destStr);
    makeFieldConstraints(f, destStr);
    makeFieldDefaultValue(f, destStr);
}

void BaseCSG::makeFieldType(const Field& f, string& destStr) const
{
    destStr += getTypeKeyword(f);
    const FieldType& type = f.type();
    if (type.length1() != FieldType::noLength) {
        destStr += "(";
        makeNumber(type.length1(), destStr);
        if (type.length2() != FieldType::noLength) {
            destStr += ",";
            makeNumber(type.length2(), destStr);
        }
        destStr += ")";
    }
}

void BaseCSG::makeFieldConstraints(const Field& f, string& destStr) const
{
    const set<Constraint>& constraints = f.constraints();
    for (set<Constraint>::const_iterator c = constraints.begin();
         c != constraints.end(); ++c) {
        string ckw = getConstraintKeyword(*c);
        if (!ckw.empty())
            destStr += " " + ckw;
    }
}

void BaseCSG::makeFieldDefaultValue(const Field& f, string& destStr) const
{
    const string& defaultValue = f.defaultValue();
    if (!defaultValue.empty())
        destStr += " DEFAULT '" + defaultValue + "'";
}

void BaseCSG::makeConstraints(const TableSchema& t, string& destStr) const
{
    makePrimaryKey(t.primaryKey(), destStr);
    const ForeignKeyList& fks = t.foreignKeys();
    for (ForeignKeyList::const_iterator i = fks.begin(); i != fks.end(); ++i)
        makeForeignKey(**i, destStr);
    makeUniqueIndices(t.uniqueFieldTuples(), destStr);
}

void BaseCSG::makePrimaryKey(const StringTuple& pk, string& destStr) const
{
    if (!pk.empty()) {
        destStr += ", PRIMARY KEY(";
        makeStringTuple(pk, destStr);
        destStr += ")";
    }
}

void BaseCSG::makeForeignKey(const ForeignKey& fk, string& destStr) const
{
    destStr += ", FOREIGN KEY(";
    makeStringTuple(fk.sourceFields(), destStr);
    destStr += ") REFERENCES ";
    destStr += fk.destinationTable();
    destStr += "(";
    makeStringTuple(fk.destinationFields(), destStr);
    destStr += ") ON DELETE ";
    destStr += getPolicyKeyword(fk.deletePolicy());
    destStr += " ON UPDATE ";
    destStr += getPolicyKeyword(fk.updatePolicy());
}

void
BaseCSG::makeUniqueIndices(const StringTupleList& ul, string& destStr) const
{
    for (StringTupleList::const_iterator u = ul.begin(); u != ul.end(); ++u) {
        destStr += ", UNIQUE(";
        makeStringTuple(**u, destStr);
        destStr += ")";
    }
}

void BaseCSG::makeIndices(const TableSchema& t, list<string>& destList) const
{
    int n = 0;
    const StringTupleList& idx = t.indexedFieldTuples();
    for (StringTupleList::const_iterator i = idx.begin();
         i != idx.end(); ++i) {
        string destStr = "CREATE INDEX " + t.name() + "_idx";
        makeNumber(++n, destStr);
        destStr += " ON " + t.name() + "(";
        makeStringTuple(**i, destStr);
        destStr += ")";
        destList.push_back(destStr);
    }
}

void BaseCSG::makeMetadataInserts(const Identifier& id,
                                  list<string>& destList) const
{
    TableSchema t("database_metadata");
    t.createField("property", FieldType(Varchar, 20));
    t.setPrimaryKey("property");
    t.createField("value", FieldType(Varchar, 255));
    makeTable(t, destList);

    string prefix = "INSERT INTO database_metadata(property, value) VALUES ";
    string destStr;

    destStr = prefix + "('name', '" + id.name() + "')";
    destList.push_back(destStr);

    destStr = prefix + "('version major', '";
    makeNumber(id.versionMajor(), destStr);
    destStr += "')";
    destList.push_back(destStr);

    destStr = prefix + "('version minor', '";
    makeNumber(id.versionMinor(), destStr);
    destStr += "')";
    destList.push_back(destStr);

    destStr = prefix + "('date year', '";
    makeNumber(id.dateYear(), destStr);
    destStr += "')";
    destList.push_back(destStr);

    destStr = prefix + "('date month', '";
    makeNumber(id.dateMonth(), destStr);
    destStr += "')";
    destList.push_back(destStr);

    destStr = prefix + "('date day', '";
    makeNumber(id.dateDay(), destStr);
    destStr += "')";
    destList.push_back(destStr);
}

void BaseCSG::makeStringTuple(const StringTuple& st, string& destStr) const
{
    for (StringTuple::const_iterator s = st.begin(); s != st.end(); ++s) {
        if (s != st.begin())
            destStr += ", ";
        destStr += *s;
    }
}

void BaseCSG::makeNumber(int n, string& destStr) const
{
    char buf[21];
    snprintf(buf, 21, "%d", n);
    destStr += buf;
}
