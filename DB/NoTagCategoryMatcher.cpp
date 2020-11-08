/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
