/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "AndCategoryMatcher.h"

#include <DB/ImageInfo.h>
#include <kpabase/Logging.h>

bool DB::AndCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    for (CategoryMatcher *subMatcher : qAsConst(mp_elements)) {
        if (!subMatcher->eval(info, alreadyMatched))
            return false;
    }
    return true;
}

void DB::AndCategoryMatcher::debug(int level) const
{
    qCDebug(DBCategoryMatcherLog, "%sAND:", qPrintable(spaces(level)));
    ContainerCategoryMatcher::debug(level + 1);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
