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

#ifndef NOTCATEGORYMATCHER_H
#define NOTCATEGORYMATCHER_H

#include "CategoryMatcher.h"

namespace DB
{

    /**
     * NegationCategoryMatcher matches, iff its child matcher does not match.
     *
     * This is not a standard ContainerCategoryMatcher, because it always has exactly one child.
     */
    class NegationCategoryMatcher :public CategoryMatcher
    {
        public:
            NegationCategoryMatcher( CategoryMatcher *child );
            virtual ~NegationCategoryMatcher();
            OVERRIDE bool eval( ImageInfoPtr, QMap<QString, StringSet>& alreadyMatched );
            OVERRIDE void debug( int level ) const;
            OVERRIDE void setShouldCreateMatchedSet( bool b );
        private:
            CategoryMatcher *_child;
    };

}

#endif /* NOTCATEGORYMATCHER_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
