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

#include "DatabaseSchema.h"
#include <stdexcept>

using namespace SQLDB::Schema;

// =======================================================================

FieldType::FieldType(DataType dataType,
                     int length1, int length2):
    _dataType(dataType),
    _length1(length1),
    _length2(length2)
{
}

DataType FieldType::dataType() const
{
    return _dataType;
}

int FieldType::length1() const
{
    return _length1;
}

int FieldType::length2() const
{
    return _length2;
}

// =======================================================================

Field::Field(const string& name, FieldType type):
    _name(name),
    _type(type),
    _characterSet(DefaultCharacterSet)
{
}

void Field::addConstraint(Constraint constraint)
{
    _constraints.insert(constraint);
}

void Field::setDefaultValue(const string& defaultValue)
{
    _defaultValue = defaultValue;
}

void Field::setCharacterSet(CharacterSet characterSet)
{
    _characterSet = characterSet;
}

const string& Field::name() const
{
    return _name;
}

const FieldType& Field::type() const
{
    return _type;
}

const set<Constraint>& Field::constraints() const
{
    return _constraints;
}

const string& Field::defaultValue() const
{
    return _defaultValue;
}

CharacterSet Field::characterSet() const
{
    return _characterSet;
}

// =======================================================================

ForeignKey::ForeignKey(const StringTuple& sourceFields):
    _srcFields(sourceFields),
    _deletePolicy(Restrict),
    _updatePolicy(Restrict)
{
}

void ForeignKey::setDestination(const string& table, const StringTuple& fields)
{
    _dstTable = table;
    _dstFields = fields;
}

void ForeignKey::setDeletePolicy(Policy policy)
{
    _deletePolicy = policy;
}

void ForeignKey::setUpdatePolicy(Policy policy)
{
    _updatePolicy = policy;
}

const StringTuple& ForeignKey::sourceFields() const
{
    return _srcFields;
}

const string& ForeignKey::destinationTable() const
{
    return _dstTable;
}

const StringTuple& ForeignKey::destinationFields() const
{
    return _dstFields;
}

Policy ForeignKey::deletePolicy() const
{
    return _deletePolicy;
}

Policy ForeignKey::updatePolicy() const
{
    return _updatePolicy;
}

// =======================================================================

TableSchema::TableSchema(const string& name):
    _name(name)
{
}

Field* TableSchema::createField(const string& name, const FieldType& type)
{
    if (!name.empty() && !hasField(name)) {
        Field* f = new Field(name, type);
        _fields.push_back(f);
        _fieldMap[name] = f;
        return f;
    }
    else
        return 0;
}

void TableSchema::setPrimaryKey(const StringTuple& fields)
{
    if (hasFields(fields))
        _primaryKey = fields;
}

ForeignKey* TableSchema::createForeignKey(const StringTuple& fields)
{
    if (!fields.empty() && hasFields(fields)) {
        ForeignKey* fk = new ForeignKey(fields);
        _foreignKeys.push_back(fk);
        return fk;
    }
    else
        return 0;
}

void TableSchema::setUnique(const StringTuple& fields)
{
    if (!fields.empty() && hasFields(fields))
        _uniqueFieldTuples.push_back(new StringTuple(fields));
}

void TableSchema::setIndexed(const StringTuple& fields)
{
    if (!fields.empty() && hasFields(fields))
        _indexedFieldTuples.push_back(new StringTuple(fields));
}

const string& TableSchema::name() const
{
    return _name;
}

const FieldList& TableSchema::fields() const
{
    return _fields;
}

const StringTuple& TableSchema::primaryKey() const
{
    return _primaryKey;
}

const ForeignKeyList& TableSchema::foreignKeys() const
{
    return _foreignKeys;
}

const StringTupleList& TableSchema::uniqueFieldTuples() const
{
    return _uniqueFieldTuples;
}

const StringTupleList& TableSchema::indexedFieldTuples() const
{
    return _indexedFieldTuples;
}

bool TableSchema::hasField(const string& name) const
{
    return _fieldMap.find(name) != _fieldMap.end();
}

bool TableSchema::hasFields(const StringTuple& fields) const
{
    for (StringTuple::const_iterator i = fields.begin();
         i != fields.end(); ++i) {
        if (!hasField(*i))
            return false;
    }
    return true;
}

// =======================================================================

Identifier::Identifier(const string& name, int versionMajor, int versionMinor):
    _name(name),
    _versionMajor(versionMajor),
    _versionMinor(versionMinor),
    _year(0),
    _month(0),
    _day(0)
{
    for (string::const_iterator i = _name.begin(); i != _name.end(); ++i)
        if ((*i < 'a' || *i > 'z') && *i != '_')
            throw std::logic_error("Invalid character in name");
}

Identifier& Identifier::setDate(int year, int month, int day)
{
    if (month < 1 || month > 12 || day < 1 || day > 31)
        throw std::logic_error("Invalid date");
    _year = year;
    _month = month;
    _day = day;
    return *this;
}

bool Identifier::isCompatibleWith(const Identifier& other) const
{
    return (_name == other._name) && (_versionMajor == other._versionMajor);
}

const string& Identifier::name() const
{
    return _name;
}

int Identifier::versionMajor() const
{
    return _versionMajor;
}

int Identifier::versionMinor() const
{
    return _versionMinor;
}

int Identifier::dateYear() const
{
    return _year;
}

int Identifier::dateMonth() const
{
    return _month;
}

int Identifier::dateDay() const
{
    return _day;
}

// =======================================================================

DatabaseSchema::DatabaseSchema(const Identifier& identifier):
    _id(identifier)
{
}

TableSchema* DatabaseSchema::createTable(const string& name)
{
    if (!name.empty() && !hasTable(name)) {
        TableSchema* t = new TableSchema(name);
        _tables.push_back(t);
        _tableMap[name] = t;
        return t;
    }
    else
        return 0;
}

const TableList& DatabaseSchema::tables() const
{
    return _tables;
}

bool DatabaseSchema::hasTable(const string& name) const
{
    return _tableMap.find(name) != _tableMap.end();
}

const Identifier& DatabaseSchema::identifier() const
{
    return _id;
}
