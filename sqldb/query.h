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
#include <optionmatcher.h>
class QSqlError;
class ImageSearchInfo;

namespace SQLDB {
    QStringList buildQueries( OptionMatcher* );
    QString buildAndQuery( OptionAndMatcher* matcher );
    QString buildValue( const QString& category, const QStringList& values, int idx, bool negate );
    QStringList filesMatchingQuery( const ::ImageSearchInfo& info );
    void showError( const QSqlError& error, const QString& query );
    QStringList values( OptionValueMatcher* matcher );
}

#endif /* SQLDB_QUERY_H */

