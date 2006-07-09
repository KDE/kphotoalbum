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


DatabaseHandler* DatabaseHandler::getMySQLHandler(const QString& username,
                                                  const QString& password,
                                                  const QString& hostname)
{
    KexiDB::Driver* driver = _driverManager->driver("MySQL");
    if (!driver) {
        // TODO: error handling (get from manager)
        exitError(i18n("Kexi driver not found."));
    }

    KexiDB::ConnectionData connectionData;
    connectionData.userName = username;
    connectionData.password = password;
    if (!hostname.isNull())
        connectionData.hostName = hostname;

    return new DatabaseHandler(driver, connectionData);
}

DatabaseHandler::DatabaseHandler(KexiDB::Driver* driver,
                                 const KexiDB::ConnectionData& connectionData):
    _driver(driver),
    _connectionData(connectionData)
{
    _connection = _driver->createConnection(_connectionData);
    if (!_connection || _driver->error()) {
        // TODO: error handling (get from driver)
        exitError(QString::fromLatin1("cannot create connection or driver error"));
    }

    if (!_connection->connect()) {
        // TODO: error handling
        exitError(QString::fromLatin1("connecting to db failed"));
    }
}

DatabaseHandler::~DatabaseHandler()
{
    delete _connection;
    delete _driver;
}

KexiDB::Connection* DatabaseHandler::connection()
{
    return _connection;
}

void DatabaseHandler::openDatabase(const QString& name)
{
    if (!_connection->databaseExists(name)) {
        createAndOpenDatabase(name);
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
    // TODO: use transaction
//     bool useTransactions = driver->transactionsSupported();
//     KexiDB::Transaction t;
//     if (useTransactions) {
//         _connection->setAutoCommit(false);
//         t = _connection->beginTransaction();
//         if (_connection->error()) {
//             qDebug("transaction failed: %s",
//                    _connection->errorMsg().latin1());
//         }
//     }

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
    f->setCaption("type");
    schema->addField(f);

    f = new KexiDB::Field("height", KexiDB::Field::Integer,
                          Field::NoConstraints,
                          Field::Unsigned);
    f->setCaption("type");
    schema->addField(f);

    f = new KexiDB::Field("angle", KexiDB::Field::ShortInteger);
    f->setCaption("type");
    schema->addField(f);

    if (!_connection->createTable(schema)) {
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

//     if (useTransactions) {
//         _connection->setAutoCommit(false);
//         if (!_connection->commitTransaction(t)) {
//             qDebug("transaction commit failed: %s",
//                    _connection->errorMsg().latin1());
//         }
//     }
}
