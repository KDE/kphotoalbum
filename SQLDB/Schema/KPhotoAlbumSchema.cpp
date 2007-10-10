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

#include "KPhotoAlbumSchema.h"

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
                (Identifier("kphotoalbum", 2, 1).setDate(2006, 11, 1));

            TableSchema* t;
            Field* f;
            ForeignKey* fk;


            // ============ dir table ============
            t = schema->createTable("dir");

            f = t->createField("id", FieldType(UnsignedInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField("path", FieldType(Varchar, 511));
            f->addConstraint(NotNull);
            //f->addConstraint(Unique);

            t->setPrimaryKey("id");


            // ============ media table ============
            t = schema->createTable("media");

            f = t->createField("id", FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField("place", FieldType(UnsignedBigInteger));

            f = t->createField("dirid", FieldType(UnsignedInteger));
            f->addConstraint(NotNull);

            f = t->createField("filename", FieldType(Varchar, 255));
            f->addConstraint(NotNull);

            f = t->createField("md5sum", FieldType(Char, 32));
            f->setCharacterSet(Ascii);

            f = t->createField("type", FieldType(UnsignedSmallInteger));
            f->addConstraint(NotNull);

            f = t->createField("label", FieldType(Varchar, 255));

            f = t->createField("description", FieldType(Text));

            f = t->createField("starttime", FieldType(Timestamp));

            f = t->createField("endtime", FieldType(Timestamp));

            f = t->createField("width", FieldType(UnsignedInteger));

            f = t->createField("height", FieldType(UnsignedInteger));

            f = t->createField("angle", FieldType(SmallInteger));

            t->setPrimaryKey("id");

            fk = t->createForeignKey("dirid");
            fk->setDestination("dir", "id");

            t->setUnique(StringTuple("dirid", "filename"));

            t->setIndexed("place");
            t->setIndexed(StringTuple("starttime", "endtime", "id"));
            t->setIndexed(StringTuple("endtime", "starttime", "id"));


            // ============ blockitem table ============
            t = schema->createTable("blockitem");

            f = t->createField("dirid", FieldType(UnsignedInteger));
            f->addConstraint(NotNull);

            f = t->createField("filename", FieldType(Varchar, 255));
            f->addConstraint(NotNull);

            t->setPrimaryKey(StringTuple("dirid", "filename"));

            fk = t->createForeignKey("dirid");
            fk->setDestination("dir", "id");


            // ============ category table ============
            t = schema->createTable("category");

            f = t->createField("id", FieldType(UnsignedInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField("name", FieldType(Varchar, 255));
            f->addConstraint(NotNull);
            f->addConstraint(Unique);

            f = t->createField("icon", FieldType(Varchar, 255));

            f = t->createField("visible", FieldType(Boolean));

            f = t->createField("viewtype", FieldType(SmallInteger));

            f = t->createField("thumbsize", FieldType(SmallInteger));

            t->setPrimaryKey("id");


            // ============ tag table ============
            t = schema->createTable("tag");

            f = t->createField("id", FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);
            f->addConstraint(AutoIncrement);

            f = t->createField("place", FieldType(UnsignedBigInteger));

            f = t->createField("categoryid", FieldType(UnsignedInteger));
            f->addConstraint(NotNull);

            f = t->createField("name", FieldType(Varchar, 255));
            f->addConstraint(NotNull);

            f = t->createField("isgroup", FieldType(Boolean));
            f->addConstraint(NotNull);
            f->setDefaultValue("0");

            t->setPrimaryKey("id");

            fk = t->createForeignKey("categoryid");
            fk->setDestination("category", "id");
            fk->setDeletePolicy(Cascade);

            t->setUnique(StringTuple("categoryid", "name"));

            t->setIndexed("place");


            // ============ media_tag table ============
            t = schema->createTable("media_tag");

            f = t->createField("mediaid", FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            f = t->createField("tagid", FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            t->setPrimaryKey(StringTuple("mediaid", "tagid"));

            fk = t->createForeignKey("mediaid");
            fk->setDestination("media", "id");
            fk->setDeletePolicy(Cascade);

            fk = t->createForeignKey("tagid");
            fk->setDestination("tag", "id");
            fk->setDeletePolicy(Cascade);

            t->setIndexed(StringTuple("tagid", "mediaid"));


            // ============ membergroup table ============
            t = schema->createTable("membergroup");

            f = t->createField("grouptag", FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            f = t->createField("membertag", FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            t->setPrimaryKey(StringTuple("grouptag", "membertag"));

            fk = t->createForeignKey("grouptag");
            fk->setDestination("tag", "id");
            fk->setDeletePolicy(Cascade);

            fk = t->createForeignKey("membertag");
            fk->setDestination("tag", "id");
            fk->setDeletePolicy(Cascade);


            // ============ drawing table ============
            t = schema->createTable("drawing");

            f = t->createField("mediaid", FieldType(UnsignedBigInteger));
            f->addConstraint(NotNull);

            f = t->createField("shape", FieldType(UnsignedSmallInteger));
            f->addConstraint(NotNull);

            f = t->createField("x0", FieldType(Integer));
            f->addConstraint(NotNull);

            f = t->createField("y0", FieldType(Integer));
            f->addConstraint(NotNull);

            f = t->createField("x1", FieldType(Integer));
            f->addConstraint(NotNull);

            f = t->createField("y1", FieldType(Integer));
            f->addConstraint(NotNull);

            fk = t->createForeignKey("mediaid");
            fk->setDestination("media", "id");
            fk->setDeletePolicy(Cascade);
        }
        return *schema;
    }
}
}
