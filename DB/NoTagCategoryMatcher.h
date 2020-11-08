/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTAGCATEGORYMATCHER_H
#define NOTAGCATEGORYMATCHER_H

#include "CategoryMatcher.h"

namespace DB
{

/**
   \brief Match pictures with no tags set for a certain category.
*/
class NoTagCategoryMatcher : public CategoryMatcher
{
public:
    explicit NoTagCategoryMatcher(const QString &category);
    ~NoTagCategoryMatcher() override;
    bool eval(ImageInfoPtr, QMap<QString, StringSet> &alreadyMatched) override;
    void debug(int level) const override;

private:
    const QString m_category;
};

}

#endif /* NOTAGCATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
