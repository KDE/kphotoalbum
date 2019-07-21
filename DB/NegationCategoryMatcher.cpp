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
#include "NegationCategoryMatcher.h"

#include "ImageInfo.h"
#include "Logging.h"

DB::NegationCategoryMatcher::NegationCategoryMatcher(CategoryMatcher *child)
    : m_child(child)
{
    Q_ASSERT(m_child);
}

DB::NegationCategoryMatcher::~NegationCategoryMatcher()
{
    delete m_child;
}
void DB::NegationCategoryMatcher::setShouldCreateMatchedSet(bool b)
{
    m_child->setShouldCreateMatchedSet(b);
}

bool DB::NegationCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    return !m_child->eval(info, alreadyMatched);
}

void DB::NegationCategoryMatcher::debug(int level) const
{
    qCDebug(DBCategoryMatcherLog, "%sNOT:", qPrintable(spaces(level)));
    m_child->debug(level + 1);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
