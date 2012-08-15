/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "ValueCategoryMatcher.h"
#include "ImageDB.h"
#include "MemberMap.h"

void DB::ValueCategoryMatcher::debug(int level) const
{
    qDebug("%s%s: %s", qPrintable(spaces(level)), qPrintable(_category), qPrintable(_option));
}

DB::ValueCategoryMatcher::ValueCategoryMatcher( const QString& category, const QString& value )
{
    _category = category ;
    _option = value;

    const MemberMap& map = DB::ImageDB::instance()->memberMap();
    const QStringList members = map.members(_category, _option, true);
    _members = members.toSet();
}

bool DB::ValueCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
    // Only add the tag _option to the alreadyMatched tags,
	// and omit the tags in _members
    if ( _shouldPrepareMatchedSet )
        alreadyMatched[_category].insert(_option);

    if ( info->hasCategoryInfo( _category, _option ) ) {
        return true;
    }

    if ( info->hasCategoryInfo( _category, _members ) )
        return true;
    return false;
}

