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
#include <kexidb/indexschema.h>
#include <kexidb/transaction.h>
#include <kexidb/field.h>
#include "DatabaseHandler.h"
#include "QueryErrors.h"

using namespace SQLDB;
using KexiDB::Field;

KexiDB::DriverManager* DatabaseHandler::_driverManager =
    new KexiDB::DriverManager();

//TODO: error handling
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>
namespace
{
    void tellError(const QString& msg)
    {
        KMessageBox::sorry(0, msg);
    }
    void exitError(const QString& msg)
    {
        tellError(msg);
        exit(-1);
    }
}

DatabaseHandler::DatabaseHandler(const KexiDB::ConnectionData& connectionData):
    _connectionData(connectionData),
    _driver(_driverManager->driver(connectionData.driverName))
{
    if (!_driver)
        throw Error(_driverManager->errorMsg());

    _connection = _driver->createConnection(_connectionData);
    if (!_connection)
        throw Error(_driver->errorMsg());

    connect();
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
        throw Error(_connection->errorMsg());
}

void DatabaseHandler::reconnect()
{
    QString usedDatabase;
    if (_connection->isConnected()) {
        usedDatabase = _connection->currentDatabase();
        bool success = _connection->disconnect();
        if (!success)
            throw Error(_connection->errorMsg());
    }
    connect();
    if (!usedDatabase.isEmpty())
        openDatabase(usedDatabase);
}

KexiDB::Connection* DatabaseHandler::connection()
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
        if (!_connection->useDatabase(name)) {
            // TODO: error handling
            qDebug("cannot use db kphotoalbum: %s",
                   _connection->errorMsg().latin1());
            exit(-1);
        }
    }
}

void DatabaseHandler::createAndOpenDatabase(const QString& name)
{
    qDebug("Creating db %s", name.latin1());
    if (!_connection->createDatabase(name)) {
        // TODO: error handling
        qDebug("create failed: %s", _connection->errorMsg().latin1());
        exit(-1);
    }
    else qDebug("create succeed");
    if (!_connection->useDatabase(name)) {
        // TODO: error handling
        exitError(QString("Cannot use db %1").arg(name));
    }

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

    //TODO: Set NotNull flags where should
    //TODO: error handling

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
        qDebug("creating dir table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
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

    // TODO: UNIQUE(dirId, filename)

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

    // KexiDB PostgreSQL driver has a bug, which prevents creating
    // table with DateTime fields with createTable, so use plain SQL
    // instead.
    if (_connectionData.driverName == "PostgreSQL") {
        _connection->executeSQL("CREATE TABLE media ("
                                "id SERIAL PRIMARY KEY, "
                                "place BIGINT, "
                                "dirid INTEGER NOT NULL, "
                                "filename CHARACTER VARYING(255) NOT NULL, "
                                "md5sum CHARACTER VARYING(32), "
                                "type SMALLINT NOT NULL, "
                                "label CHARACTER VARYING(255), "
                                "description TEXT, "
                                "starttime TIMESTAMP, "
                                "endtime TIMESTAMP, "
                                "width INTEGER, "
                                "height INTEGER, "
                                "angle SMALLINT)");
    }
    else if (!_connection->createTable(schema)) {
        qDebug("creating media table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


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

    // TODO: UNIQUE(dirId, filename)

    if (!_connection->createTable(schema)) {
        qDebug("creating blockitem table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


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

    f = new KexiDB::Field("viewsize", KexiDB::Field::ShortInteger);
    f->setCaption("view size");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        qDebug("creating category table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
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
                          Field::NoConstraints, Field::NoOptions, 255);
    f->setCaption("name");
    schema->addField(f);

    // TODO: UNIQUE(categoryId, name)

    f = new KexiDB::Field("isGroup", KexiDB::Field::Boolean,
                          Field::NotNull);
    f->setCaption("is member group");
    f->setDefaultValue(QVariant(0));
    schema->addField(f);

    if (!_connection->createTable(schema)) {
        qDebug("creating tag table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== media_tag table ====
    schema = new KexiDB::TableSchema("media_tag");
    schema->setCaption("media item tags");
    // TODO: create index
    //KexiDB::IndexSchema *indexSchema = new KexiDB::IndexSchema(schema);

    f = new Field("mediaId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("media item id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (mediaId) REFERENCES media(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT
    // FOREIGN KEY (tagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new Field("tagId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("tag id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (tagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    // TODO: UNIQUE(mediaId, tagId)
    //schema->addIndex(indexSchema);
    //schema->setPrimaryKey(indexSchema);

    if (!_connection->createTable(schema)) {
        qDebug("creating media_tag table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }


    // ==== tag_relation table ====
    schema = new KexiDB::TableSchema("tag_relation");
    schema->setCaption("tag relations");
    // TODO: create index
    //indexSchema = new KexiDB::IndexSchema(schema);

    f = new Field("toTagId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("media item id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (toTagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    f = new Field("fromTagId", Field::BigInteger,
                  Field::ForeignKey | Field::NotNull, Field::Unsigned);
    f->setCaption("tag id");
    schema->addField(f);
    //indexSchema->addField(f);

    // TODO: foreign key constraint:
    // FOREIGN KEY (fromTagId) REFERENCES tag(id)
    // ON DELETE CASCADE ON UPDATE RESTRICT

    // TODO: UNIQUE(toTagId, fromTagId)
    //schema->addIndex(indexSchema);
    //schema->setPrimaryKey(indexSchema);

    if (!_connection->createTable(schema)) {
        qDebug("creating tag_relation table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }

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
        qDebug("creating drawing table failed: %s",
               _connection->errorMsg().latin1());
        delete schema;
    }

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
    _connection->executeSQL("INSERT INTO category "
                            "(name, icon, visible, viewtype, viewsize) "
                            "VALUES "
                            "('Folder', 'folder', 0, 0, 0), "
                            "('Tokens', 'cookie', 1, 1, 1), "
                            "('Keywords', 'password', 1, 1, 1), "
                            "('Locations', 'network', 1, 0, 0), "
                            "('Persons', 'personal', 1, 0, 0)");
}
