/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VALUECATEGORYMATCHER_H
#define VALUECATEGORYMATCHER_H

#include "SimpleCategoryMatcher.h"

namespace DB
{

class ValueCategoryMatcher : public SimpleCategoryMatcher
{
public:
    ValueCategoryMatcher(const QString &category, const QString &value);
    bool eval(ImageInfoPtr, QMap<QString, StringSet> &alreadyMatched) override;
    void debug(int level) const override;

    QString m_option;
    StringSet m_members;
};

}

#endif /* VALUECATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
