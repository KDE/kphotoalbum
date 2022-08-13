/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEGATIONCATEGORYMATCHER_H
#define NEGATIONCATEGORYMATCHER_H

#include "CategoryMatcher.h"

namespace DB
{

/**
 * NegationCategoryMatcher matches, if (and only if) its child matcher does not match.
 *
 * This is not a standard ContainerCategoryMatcher, because it always has exactly one child.
 */
class NegationCategoryMatcher : public CategoryMatcher
{
public:
    explicit NegationCategoryMatcher(CategoryMatcher *child);
    ~NegationCategoryMatcher() override;
    bool eval(ImageInfoPtr, QMap<QString, StringSet> &alreadyMatched) override;
    void debug(int level) const override;
    void setShouldCreateMatchedSet(bool b) override;

private:
    CategoryMatcher *m_child;
};
}

#endif // NEGATIONCATEGORYMATCHER_H

// vi:expandtab:tabstop=4 shiftwidth=4:
