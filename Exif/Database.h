/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef EXIFDATABASE_H
#define EXIFDATABASE_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qpair.h>
#include "Utilities/Set.h"

class QSqlDatabase;
class QSqlQuery;

namespace Exiv2 {
class ExifData;
}

typedef QPair<int,int> Rational;
typedef QValueList<Rational> RationalList;

namespace Exif
{

using Utilities::StringSet;

// ================================================================================
// IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT
// ================================================================================
//
// It is the resposibility of the methods in here to bail out in case database support
// is not available ( !isAvailable() ). This is to simplify client code.
class Database {

public:
    static Database* instance();
    static bool isAvailable();

    bool isOpen() const;
    bool isUsable() const;
    void add( const QString& fileName );
    void remove( const QString& fileName );
    StringSet filesMatchingQuery( const QString& query );
    QValueList< QPair<QString,QString> > cameras() const;

protected:
    static QString exifDBFile();
    void openDatabase();
    void populateDatabase();
    static QString connectionName();
    void insert( const QString& filename, Exiv2::ExifData );
    void offerInitialize();

private:
    bool _isOpen;
    bool _doUTF8Conversion;
    Database();
    void init();
    static Database* _instance;
    QSqlDatabase* _db;
};

}

#endif /* EXIFDATABASE_H */

