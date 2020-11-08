/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ANDCATEGORYMATCHER_H
#define ANDCATEGORYMATCHER_H

#include "ContainerCategoryMatcher.h"

namespace DB
{

class AndCategoryMatcher : public ContainerCategoryMatcher
{
public:
    bool eval(ImageInfoPtr, QMap<QString, StringSet> &alreadyMatched) override;
    void debug(int level) const override;
};

}

#endif /* ANDCATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
