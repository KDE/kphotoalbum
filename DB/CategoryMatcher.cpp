/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CategoryMatcher.h"

using namespace DB;

CategoryMatcher::CategoryMatcher()
    : m_shouldPrepareMatchedSet(false)
{
}

QString CategoryMatcher::spaces(int level) const
{
    return QString::fromLatin1("").rightJustified(level * 3);
}

void DB::CategoryMatcher::setShouldCreateMatchedSet(bool b)
{
    m_shouldPrepareMatchedSet = b;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
