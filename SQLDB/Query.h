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

#ifndef SQLDB_QUERY_H
#define SQLDB_QUERY_H
#include "DB/CategoryMatcher.h"
#include <qvariant.h>
class QSqlQuery;
class QSqlError;

namespace DB
{
    class ImageSearchInfo;
}


namespace SQLDB {
    QValueList<int> filesMatchingQuery( const DB::ImageSearchInfo& info );
    QValueList<int> runCategoryQuery( QValueList<DB::OptionSimpleMatcher*> );
    void split( const QValueList<DB::OptionSimpleMatcher*>& input,
                QValueList<DB::OptionSimpleMatcher*>& positiveList,
                QValueList<DB::OptionSimpleMatcher*>& negativeList );
    QString buildQueryPrefix( int count, int firstId );

    QString buildValue( const QString& category, const QStringList& values, int idx, bool negate );
    QStringList values( DB::OptionValueMatcher* matcher );

    QValueList<int> mergeUniqly( QValueList<int>, QValueList<int> );
    QValueList<int> listSubstract( QValueList<int>, QValueList<int> );

}

#endif /* SQLDB_QUERY_H */

