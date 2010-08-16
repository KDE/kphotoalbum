/*
  Copyright (C) 2007-2010 Tuomas Suutari <thsuut@utu.fi>

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

#include "KPhotoAlbumSchema.h"

// Used around database table and field names to searching of those
// identifiers easier
#define DBN(x) x

namespace SQLDB {
namespace Schema {
    const DatabaseSchema& getKPhotoAlbumSchema()
    {
        static DatabaseSchema* schema(0);
        if (!schema) {
            //
            // Remember to update minor version and date every time
            // you make a change! Also major should be increased and
            // minor set to 0 when making incompatible change.
            //
            schema =
                new DatabaseSchema
                (Identifier("kphotoalbum", 5, 0).setDate(2008, 9, 5));

            TableSchema* t;
            Field* f;
            ForeignKey* fk;


            // ============ directory table ============
            t = schema->createTable(DBN("directory"));

            f = t->createField(DBN("id"), FieldType(UnsignedInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField(DBN("path"), FieldType(Varchar, 511));
            f->addConstraint(NotNull);
            //f->addConstraint(Unique);

            t->setPrimaryKey(DBN("id"));


            // ============ file table ============
            t = schema->createTable(DBN("file"));

            f = t->createField(DBN("id"), FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField(DBN("position"), FieldType(UnsignedBigInteger));

            f = t->createField(DBN("directory_id"), FieldType(UnsignedInteger));
            f->addConstraint(NotNull);

            f = t->createField(DBN("filename"), FieldType(Varchar, 255));
            f->addConstraint(NotNull);

            f = t->createField(DBN("md5sum"), FieldType(Char, 32));
            f->setCharacterSet(Ascii);

            f = t->createField(DBN("type"), FieldType(UnsignedSmallInteger));
            f->addConstraint(NotNull);

            f = t->createField(DBN("label"), FieldType(Varchar, 255));

            f = t->createField(DBN("description"), FieldType(Text));

            f = t->createField(DBN("time_start"), FieldType(Timestamp));

            f = t->createField(DBN("time_end"), FieldType(Timestamp));

            f = t->createField(DBN("width"), FieldType(UnsignedInteger));

            f = t->createField(DBN("height"), FieldType(UnsignedInteger));

            f = t->createField(DBN("angle"), FieldType(SmallInteger));

            f = t->createField(DBN("rating"), FieldType(TinyInteger));

            f = t->createField(DBN("stack_id"), FieldType(UnsignedBigInteger));
            f = t->createField(DBN("stack_position"), FieldType(UnsignedInteger));

            f = t->createField(DBN("gps_longitude"), FieldType(Double));
            f = t->createField(DBN("gps_latitude"), FieldType(Double));
            f = t->createField(DBN("gps_altitude"), FieldType(Double));
            f = t->createField(DBN("gps_precision"), FieldType(Integer));

            t->setPrimaryKey(DBN("id"));

            fk = t->createForeignKey(DBN("directory_id"));
            fk->setDestination(DBN("directory"), DBN("id"));

            t->setUnique(StringTuple(DBN("directory_id"), DBN("filename")));

            t->setIndexed(DBN("position"));
            t->setIndexed(StringTuple(DBN("time_start"), DBN("time_end"), DBN("id")));
            t->setIndexed(StringTuple(DBN("time_end"), DBN("time_start"), DBN("id")));


            // ============ ignored_file table ============
            t = schema->createTable(DBN("ignored_file"));

            f = t->createField(DBN("directory_id"), FieldType(UnsignedInteger));
            f->addConstraint(NotNull);

            f = t->createField(DBN("filename"), FieldType(Varchar, 255));
            f->addConstraint(NotNull);

            t->setPrimaryKey(StringTuple(DBN("directory_id"), DBN("filename")));

            fk = t->createForeignKey(DBN("directory_id"));
            fk->setDestination(DBN("directory"), DBN("id"));


            // ============ category table ============
            t = schema->createTable(DBN("category"));

            f = t->createField(DBN("id"), FieldType(UnsignedInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField(DBN("name"), FieldType(Varchar, 255));
            f->addConstraint(NotNull);
            f->addConstraint(Unique);

            f = t->createField(DBN("icon"), FieldType(Varchar, 255));

            f = t->createField(DBN("visible"), FieldType(Boolean));

            f = t->createField(DBN("viewtype"), FieldType(SmallInteger));

            f = t->createField(DBN("thumbsize"), FieldType(SmallInteger));

            t->setPrimaryKey(DBN("id"));


            // ============ tag table ============
            t = schema->createTable(DBN("tag"));

            f = t->createField(DBN("id"), FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField(DBN("position"), FieldType(UnsignedBigInteger));

            f = t->createField(DBN("category_id"), FieldType(UnsignedInteger));
            f->addConstraint(NotNull);

            f = t->createField(DBN("name"), FieldType(Varchar, 255));
            f->addConstraint(NotNull);

            f = t->createField(DBN("isgroup"), FieldType(Boolean));
            f->addConstraint(NotNull);
            f->setDefaultValue("0");

            t->setPrimaryKey(DBN("id"));

            fk = t->createForeignKey(DBN("category_id"));
            fk->setDestination(DBN("category"), DBN("id"));
            fk->setDeletePolicy(Cascade);

            t->setUnique(StringTuple(DBN("category_id"), DBN("name")));

            t->setIndexed(DBN("position"));


            // ============ file_tag table ============
            t = schema->createTable(DBN("file_tag"));

            f = t->createField(DBN("file_id"), FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            f = t->createField(DBN("tag_id"), FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            t->setPrimaryKey(StringTuple(DBN("file_id"), DBN("tag_id")));

            fk = t->createForeignKey(DBN("file_id"));
            fk->setDestination(DBN("file"), DBN("id"));
            fk->setDeletePolicy(Cascade);

            fk = t->createForeignKey(DBN("tag_id"));
            fk->setDestination(DBN("tag"), DBN("id"));
            fk->setDeletePolicy(Cascade);

            t->setIndexed(StringTuple(DBN("tag_id"), DBN("file_id")));


            // ============ tag_member table ============
            t = schema->createTable(DBN("tag_member"));

            f = t->createField(DBN("tag_id"), FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            f = t->createField(DBN("member_tag_id"), FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            t->setPrimaryKey(StringTuple(DBN("tag_id"), DBN("member_tag_id")));

            fk = t->createForeignKey(DBN("tag_id"));
            fk->setDestination(DBN("tag"), DBN("id"));
            fk->setDeletePolicy(Cascade);

            fk = t->createForeignKey(DBN("member_tag_id"));
            fk->setDestination(DBN("tag"), DBN("id"));
            fk->setDeletePolicy(Cascade);
        }
        return *schema;
    }
}
}
