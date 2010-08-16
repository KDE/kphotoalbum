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
#include "NoOtherItemsCategoryMatcher.h"
#include "ImageInfo.h"

DB::NoOtherItemsCategoryMatcher::NoOtherItemsCategoryMatcher( const QString& category, bool sign )
{
    _category = category;
    _sign = sign;
}

bool DB::NoOtherItemsCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
    Q_ASSERT( _shouldPrepareMatchedSet );
    bool allMatched = true;
    Q_FOREACH(const QString& item, info->itemsOfCategory(_category))
    {
        if (!alreadyMatched[_category].contains(item))
        {
            allMatched = false;
            break;
        }
    }
    return _sign ? allMatched : !allMatched;
}

void DB::NoOtherItemsCategoryMatcher::debug( int level ) const
{
    qDebug("%s%s:EMPTY", qPrintable(spaces(level)), qPrintable(_category) );
}

bool DB::NoOtherItemsCategoryMatcher::hasEmptyMatcher() const
{
    return true;
}

