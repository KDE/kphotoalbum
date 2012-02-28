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
#include "ExactCategoryMatcher.h"
#include "ImageInfo.h"

DB::ExactCategoryMatcher::ExactCategoryMatcher( const QString category)
    : _category(category), _matcher(0)
{
}

DB::ExactCategoryMatcher::~ExactCategoryMatcher()
{
    if ( _matcher )
    {
        delete _matcher;
        _matcher = 0;
    }
}

void DB::ExactCategoryMatcher::setMatcher( CategoryMatcher* subMatcher )
{
    _matcher = subMatcher;
    if ( _matcher )
        // always collect matched tags of _matcher:
        _matcher->setShouldCreateMatchedSet( true );
}

bool DB::ExactCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
    // it makes no sense to put one ExactCategoryMatcher into another, so we ignore alreadyMatched.
    Q_UNUSED( alreadyMatched );

    if ( ! _matcher )
        return false;

    QMap<QString, StringSet> matchedTags;

    // first, do a regular match and collect all matched Tags.
    if (!_matcher->eval(info, matchedTags))
        return false;

    // if the match succeeded, check if it is exact:
    Q_FOREACH(const QString& item, info->itemsOfCategory(_category))
        if ( !matchedTags[_category].contains(item) )
            return false; // tag was not contained in matcher
    return true;
}

void DB::ExactCategoryMatcher::debug( int level ) const
{
    qDebug("%sEXACT:", qPrintable(spaces(level)) );
    _matcher->debug( level + 1 );
}

void DB::ExactCategoryMatcher::setShouldCreateMatchedSet(bool)
{
    // no-op:
    // shouldCreateMatchedSet is already set to true for _matcher;
    // setting this to false would disable the ExactCategoryMatcher, so it is ignored.

    // only ExactCategoryMatcher ever calls setShouldCreateMatchedSet.
    // ExactCategoryMatcher are never stacked, so this can't be called.
    Q_ASSERT( false );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
