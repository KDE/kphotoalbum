/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ContainerCategoryMatcher.h"

void DB::ContainerCategoryMatcher::addElement(CategoryMatcher *element)
{
    mp_elements.append(element);
}

DB::ContainerCategoryMatcher::~ContainerCategoryMatcher()
{
    for (int i = 0; i < mp_elements.count(); ++i)
        delete mp_elements[i];
}

void DB::ContainerCategoryMatcher::debug(int level) const
{
    for (QList<CategoryMatcher *>::ConstIterator it = mp_elements.begin(); it != mp_elements.end(); ++it) {
        (*it)->debug(level);
    }
}

void DB::ContainerCategoryMatcher::setShouldCreateMatchedSet(bool b)
{
    m_shouldPrepareMatchedSet = b;
    for (DB::CategoryMatcher *matcher : mp_elements)
        matcher->setShouldCreateMatchedSet(b);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
