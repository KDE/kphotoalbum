/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
