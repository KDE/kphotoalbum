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

#ifndef EXACTCATEGORYMATCHER_H
#define EXACTCATEGORYMATCHER_H

#include "CategoryMatcher.h"

namespace DB
{

/**
   \brief Enforce exact matching for another matcher.

   The ExactCategoryMatcher matches, iff the sub-matcher matches and
   the ImageInfoPtr does not contain any other tags in the category.

   If no sub-matcher is set, no image is matched.

   Example:
   The sub-matcher matches images with the tags "Jesper" and "Jim" in the category "People".
   Then the ExactMatcher will match the same images, except those who are also tagged with
   any other people.

   This is a replacement for the NoOtherItemsCategoryMatcher.
*/
class ExactCategoryMatcher : public CategoryMatcher
{
public:
    explicit ExactCategoryMatcher(const QString category);
    ~ExactCategoryMatcher() override;
    void setMatcher(CategoryMatcher *subMatcher);
    bool eval(ImageInfoPtr, QMap<QString, StringSet> &alreadyMatched) override;
    void debug(int level) const override;
    /// shouldCreateMatchedSet is _always_ set for the sub-matcher of ExactCategoryMatcher.
    void setShouldCreateMatchedSet(bool) override;

private:
    const QString m_category;
    CategoryMatcher *m_matcher;
};

}

#endif /* EXACTCATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
