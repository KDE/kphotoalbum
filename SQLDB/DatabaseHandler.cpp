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

#include <kexidb/drivermanager.h>
#include <kexidb/driver.h>
#include <kexidb/connectiondata.h>
#include <kexidb/connection.h>
#include <kexidb/tableschema.h>
#include <kexidb/transaction.h>
#include <kexidb/field.h>
#include <kexidb/dbproperties.h>
#include <klocale.h>
#include "DatabaseHandler.h"
#include "QueryHelper.h"
#include "QueryErrors.h"

// Update this when making incompatible schema change
#define SCHEMA_VERSION_MAJOR 2

// Update these every time the database schema changes
#define SCHEMA_VERSION_MINOR 1
#define SCHEMA_DATE "2006-11-01"


using namespace SQLDB;
using KexiDB::Field;

KexiDB::DriverManager* DatabaseHandler::_driverManager =
    new KexiDB::DriverManager();

DatabaseHandler::DatabaseHandler(const DatabaseAddress& address):
    _databaseName(address.databaseName()),
    _connectionData(address.connectionData()),
    _driver(_driverManager->driver(_connectionData.driverName))
{
    if (!_driver)
        throw DriverLoadError(_driverManager->errorMsg());

    _connection = _driver->createConnection(_connectionData);
    if (!_connection) {
        delete _driver;
        throw ConnectionCreateError(_driver->errorMsg());
    }

    try {
        connect();
        openDatabase(_databaseName);
    }
    catch (...) {
        delete _connection;
        delete _driver;
        throw;
    }
}

DatabaseHandler::~DatabaseHandler()
{
    delete _connection;
    delete _driver;
}

void DatabaseHandler::connect()
{
    bool success = _connection->connect();
    if (!success)
        throw ConnectionOpenError(_connection->errorMsg());
}

void DatabaseHandler::reconnect()
{
    if (_connection->isConnected()) {
        bool success = _connection->disconnect();
        if (!success)
            throw ConnectionCloseError(_connection->errorMsg());
    }
    connect();
    openDatabase(_databaseName);
}

Connection* DatabaseHandler::connection()
{
    return _connection;
}

void DatabaseHandler::openDatabase(const QString& name)
{
    if (!_connection->databaseExists(name)) {
        createAndOpenDatabase(name);
        insertInitialData();
    }
    else {
        if (!_connection->useDatabase(name))
            throw DatabaseOpenError(_connection->errorMsg());

        QVariant version =
            _connection->databaseProperties().value("schema version major");
        if (version.isNull())
            throw DatabaseSchemaError(i18n("Database schema is incompatible."));
        if (version.toUInt() != SCHEMA_VERSION_MAJOR)
            throw DatabaseSchemaError(i18n("Database version is incompatible."));
    }
}

void DatabaseHandler::createAndOpenDatabase(const QString& name)
{
    if (!_connection->createDatabase(name))
        throw DatabaseCreateError(_connection->errorMsg());
    if (!_connection->useDatabase(name))
        throw DatabaseOpenError(_connection->errorMsg());

    bool useTransactions = _driver->transactionsSupported();
    KexiDB::Transaction transaction;
    if (useTransactions) {
        _connection->setAutoCommit(false);
        qDebug("beginTransaction");
        transaction = _connection->beginTransaction();
        if (_connection->error()) {
            // TODO: error handling
            qDebug("transaction failed: %s",
                   _connection->errorMsg().latin1());
        }
    }

    KexiDB::Field* f;
    KexiDB::TableSchema* schema;

    // TODO: Use KexiDB to create indices, when it supports that (IndexSchema)

    // ==== dir table ====
    schema = new KexiDB::TableSchema("dir");
    schema->setCaption("directories");

    f = new KexiDB::Field("id", Field::Integer,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("path", Field::Text,
                          Field::NotNull /* | Field::Unique */,
                          Field::NoOptions, 511);
    f->setCaption("path");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }


    // ==== media table ====
    schema = new KexiDB::TableSchema("media");
    schema->setCaption("media items");

    f = new KexiDB::Field("id", Field::BigInteger,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("place", Field::BigInteger,
                          Field::Indexed, Field::Unsigned);
    f->setCaption("place");
    schema->addField(f);

    f = new KexiDB::Field("dirId", Field::Integer,
                          Field::ForeignKey | Field::NotNull,
                          Field::Unsigned);
    f->setCaption("directory id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (dirId) REFERENCES dir(id)
    // ON DELETE RESTRICT ON UPDATE RESTRICT

    f = new KexiDB::Field("filename", KexiDB::Field::Text,
                          Field::NotNull, Field::NoOptions, 255);
    f->setCaption("filename");
    schema->addField(f);

    f = new KexiDB::Field("md5sum", KexiDB::Field::Text,
                          Field::NoConstraints, Field::NoOptions, 32);
    f->setCaption("md5sum");
    schema->addField(f);

    f = new KexiDB::Field("type", KexiDB::Field::ShortInteger,
                          Field::NotNull, Field::Unsigned);
    f->setCaption("type");
    schema->addField(f);

    f = new KexiDB::Field("label", KexiDB::Field::Text,
                          Field::NoConstraints, Field::NoOptions, 255);
    f->setCaption("label");
    schema->addField(f);

    f = new KexiDB::Field("description", KexiDB::Field::LongText);
    f->setCaption("description");
    schema->addField(f);

    f = new KexiDB::Field("startTime", KexiDB::Field::DateTime);
    f->setCaption("start time");
    schema->addField(f);

    f = new KexiDB::Field("endTime", KexiDB::Field::DateTime);
    f->setCaption("end time");
    schema->addField(f);

    f = new KexiDB::Field("width", KexiDB::Field::Integer,
                          Field::NoConstraints,
                          Field::Unsigned);
    f->setCaption("width");
    schema->addField(f);

    f = new KexiDB::Field("height", KexiDB::Field::Integer,
                          Field::NoConstraints,
                          Field::Unsigned);
    f->setCaption("height");
    schema->addField(f);

    f = new KexiDB::Field("angle", KexiDB::Field::ShortInteger);
    f->setCaption("angle");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }

    _connection->executeSQL("CREATE UNIQUE INDEX dirfileidx "
                            "ON media (dirId, filename)");
    _connection->executeSQL("CREATE INDEX timeidx1 "
                            "ON media (startTime, endTime, id)");
    _connection->executeSQL("CREATE INDEX timeidx2 "
                            "ON media (endTime, startTime, id)");


    // ==== blockitem table ====
    schema = new KexiDB::TableSchema("blockitem");
    schema->setCaption("block items");

    f = new KexiDB::Field("dirId", Field::Integer,
                          Field::ForeignKey | Field::NotNull,
                          Field::Unsigned);
    f->setCaption("directory id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (dirId) REFERENCES dir(id)
    // ON DELETE RESTRICT ON UPDATE RESTRICT

    f = new KexiDB::Field("filename", KexiDB::Field::Text,
                          Field::NotNull, Field::NoOptions, 255);
    f->setCaption("filename");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }

    _connection->executeSQL("CREATE UNIQUE INDEX blockdirfileidx "
                            "ON blockitem (dirId, filename)");


    // ==== category table ====
    schema = new KexiDB::TableSchema("category");
    schema->setCaption("categories");

    f = new KexiDB::Field("id", Field::Integer,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("name", Field::Text,
                          Field::NotNull | Field::Unique,
                          Field::NoOptions, 255);
    f->setCaption("name");
    schema->addField(f);

    f = new KexiDB::Field("icon", Field::Text,
                          Field::NoConstraints, Field::NoOptions, 1023);
    f->setCaption("name");
    schema->addField(f);

    f = new KexiDB::Field("visible", KexiDB::Field::Boolean);
    f->setCaption("is visible");
    schema->addField(f);

    f = new KexiDB::Field("viewtype", KexiDB::Field::ShortInteger);
    f->setCaption("view type");
    schema->addField(f);

    f = new KexiDB::Field("thumbsize", KexiDB::Field::ShortInteger);
    f->setCaption("thumbnail size");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }


    // ==== tag table ====
    schema = new KexiDB::TableSchema("tag");
    schema->setCaption("tags");

    f = new KexiDB::Field("id", Field::BigInteger,
                          Field::PrimaryKey | Field::AutoInc,
                          Field::Unsigned);
    f->setCaption("id");
    schema->addField(f);

    f = new KexiDB::Field("place", Field::BigInteger,
                          Field::Indexed, Field::Unsigned);
    f->setCaption("place");
    schema->addField(f);

    f = new KexiDB::Field("categoryId", Field::Integer,
                          Field::ForeignKey | Field::NotNull,
                          Field::Unsigned);
    f->setCaption("category id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (categoryId) REFERENCES category(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new KexiDB::Field("name", Field::Text,
                          Field::NotNull, Field::NoOptions, 255);
    f->setCaption("name");
    schema->addField(f);

    f = new KexiDB::Field("isGroup", KexiDB::Field::Boolean,
                          Field::NotNull);
    f->setCaption("is member group");
    f->setDefaultValue(QVariant(0));
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }

    _connection->executeSQL("CREATE UNIQUE INDEX tagctgrynameidx "
                            "ON tag (categoryId, name)");

    // ==== media_tag table ====
    schema = new KexiDB::TableSchema("media_tag");
    schema->setCaption("media item tags");

    f = new Field("mediaId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("media item id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (mediaId) REFERENCES media(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT
    // FOREIGN KEY (tagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new Field("tagId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("tag id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (tagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }

    _connection->executeSQL("CREATE UNIQUE INDEX mediatagidx "
                            "ON media_tag (mediaId, tagId)");
    _connection->executeSQL("CREATE INDEX tagmediaidx "
                            "ON media_tag (tagId, mediaId)");


    // ==== membergroup table ====
    schema = new KexiDB::TableSchema("membergroup");
    schema->setCaption("member group configuration");

    f = new Field("groupTag", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("tag id of the group");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (groupTag) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new Field("memberTag", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("tag id of the member of the group");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (memberTag) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    // groupTag and memberTag should be in same category

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }

    _connection->executeSQL("CREATE INDEX membergroupidx "
                            "ON membergroup (groupTag, memberTag)");


    // ==== drawing table ====
    schema = new KexiDB::TableSchema("drawing");
    schema->setCaption("drawings");

    f = new Field("mediaId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("media item id");
    schema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (mediaId) REFERENCES media(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new Field("shape", Field::ShortInteger,
                  Field::NotNull, Field::Unsigned);
    f->setCaption("shape type");
    schema->addField(f);

    f = new Field("x0", Field::Integer,
                  Field::NotNull);
    f->setCaption("first point x coordinate");
    schema->addField(f);

    f = new Field("y0", Field::Integer,
                  Field::NotNull);
    f->setCaption("first point y coordinate");
    schema->addField(f);

    f = new Field("x1", Field::Integer,
                  Field::NotNull);
    f->setCaption("second point x coordinate");
    schema->addField(f);

    f = new Field("y1", Field::Integer,
                  Field::NotNull);
    f->setCaption("second point y coordinate");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        delete schema;
        throw TableCreateError(_connection->errorMsg());
    }


    KexiDB::DatabaseProperties& properties = _connection->databaseProperties();
    properties.setValue("schema version major", SCHEMA_VERSION_MAJOR);
    properties.setValue("schema version minor", SCHEMA_VERSION_MINOR);
    properties.setValue("schema date", SCHEMA_DATE);


    if (useTransactions) {
        _connection->setAutoCommit(false);
        qDebug("commitTransaction");
        if (!_connection->commitTransaction(transaction)) {
            qDebug("transaction commit failed: %s",
                   _connection->errorMsg().latin1());
        }
    }
}

void DatabaseHandler::insertInitialData()
{
    struct
    {
        const char* name;
        const char* icon;
        bool visible;
        DB::Category::ViewType viewtype;
        int thumbnailSize;
    } entry[] = {
        { "Tokens", "cookie",
          true, DB::Category::IconView, 32 },
        { "Keywords", "password",
          true, DB::Category::IconView, 32 },
        { "Places", "network",
          true, DB::Category::ListView, 32 },
        { "People", "personal",
          true, DB::Category::ThumbedListView, 96 },
        { 0, 0, false, DB::Category::ListView, 0 }
    };

    QueryHelper qh(*_connection);

    for (int i = 0; entry[i].name != 0; ++i)
        qh.insertCategory(QString::fromLatin1(entry[i].name),
                          QString::fromLatin1(entry[i].icon),
                          entry[i].visible,
                          entry[i].viewtype, entry[i].thumbnailSize);
}
