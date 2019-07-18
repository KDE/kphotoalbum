/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "NoTagCategoryMatcher.h"
#include "ImageInfo.h"
#include "Logging.h"

DB::NoTagCategoryMatcher::NoTagCategoryMatcher(const QString &category)
    : m_category(category)
{
}

DB::NoTagCategoryMatcher::~NoTagCategoryMatcher()
{
}

bool DB::NoTagCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    Q_UNUSED(alreadyMatched);
    return info->itemsOfCategory(m_category).isEmpty();
}

void DB::NoTagCategoryMatcher::debug(int level) const
{
    qCDebug(DBCategoryMatcherLog) << qPrintable(spaces(level)) << "No Tags for category " << m_category;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
