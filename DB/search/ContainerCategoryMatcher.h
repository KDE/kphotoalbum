/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTAINERCATEGORYMATCHER_H
#define CONTAINERCATEGORYMATCHER_H

#include "CategoryMatcher.h"

namespace DB
{

class ContainerCategoryMatcher : public CategoryMatcher
{
public:
    void addElement(CategoryMatcher *);
    ~ContainerCategoryMatcher() override;
    void debug(int level) const override;
    void setShouldCreateMatchedSet(bool) override;

    QList<CategoryMatcher *> mp_elements;
};

}

#endif /* CONTAINERCATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
