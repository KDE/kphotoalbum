/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "NegationCategoryMatcher.h"

#include <DB/ImageInfo.h>
#include <kpabase/Logging.h>

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
