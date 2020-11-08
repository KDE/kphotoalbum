/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SIMPLECATEGORYMATCHER_H
#define SIMPLECATEGORYMATCHER_H

#include "CategoryMatcher.h"

namespace DB
{

class SimpleCategoryMatcher : public CategoryMatcher
{
public:
    QString m_category;
};

}

#endif /* SIMPLECATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
