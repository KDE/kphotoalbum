/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef SQLDB_QUERYUTIL_H
#define SQLDB_QUERYUTIL_H
#include "DB/CategoryMatcher.h"
#include <qvariant.h>
class QSqlQuery;
class QSqlError;

namespace DB
{
    class ImageSearchInfo;
}

namespace SQLDB {
    void showError( QSqlQuery& query );
    QStringList runAndReturnList( const QString& queryString, const QMap<QString,QVariant>& bindings = QMap<QString,QVariant>() );
    QValueList<int> runAndReturnIntList( const QString& queryString, const QMap<QString,QVariant>& bindings = QMap<QString,QVariant>() );
    QVariant fetchItem( const QString& queryString, const QMap<QString,QVariant>& bindings  = QMap<QString,QVariant>() );
    bool runQuery( const QString& queryString, const QMap<QString,QVariant>& bindings, QSqlQuery& query  );
    bool runQuery( const QString& queryString, const QMap<QString,QVariant>& bindings );
    QStringList membersOfCategory( const QString& category );
    QString fileNameForId( int id, bool fullPath );
    int idForFileName( const QString& fullPath );
    QString categoryForId( int id );
    int idForCategory( const QString& category );
    QValueList<int> allImages();
}

#endif /* SQLDB_QUERYUTIL_H */

