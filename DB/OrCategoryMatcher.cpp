/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "OrCategoryMatcher.h"

#include "ImageInfo.h"
#include "Logging.h"

bool DB::OrCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    for (CategoryMatcher *subMatcher : mp_elements) {
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
