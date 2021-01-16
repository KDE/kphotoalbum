/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "OrCategoryMatcher.h"

#include "ImageInfo.h"

#include <kpabase/Logging.h>

bool DB::OrCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    for (CategoryMatcher *subMatcher : qAsConst(mp_elements)) {
        if (subMatcher->eval(info, alreadyMatched))
            return true;
    }
    return false;
}

void DB::OrCategoryMatcher::debug(int level) const
{
    qCDebug(DBCategoryMatcherLog, "%sOR:", qPrintable(spaces(level)));
    ContainerCategoryMatcher::debug(level + 1);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
