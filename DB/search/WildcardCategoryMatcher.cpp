// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "WildcardCategoryMatcher.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <kpabase/Logging.h>

void DB::WildcardCategoryMatcher::debug(int level) const
{
    qCDebug(DBCategoryMatcherLog, "%s: %s", qPrintable(spaces(level)), qPrintable(m_re.pattern()));
}

QRegularExpression DB::WildcardCategoryMatcher::regularExpression() const
{
    return m_re;
}

void DB::WildcardCategoryMatcher::setRegularExpression(const QRegularExpression &re)
{
    m_re = re;
    m_matchingTags.clear();
    for (const auto &category : ImageDB::instance()->categoryCollection()->categories()) {
        if (category->isSpecialCategory())
            continue;
        for (const auto &tag : category->itemsInclCategories()) {
            if (m_re.match(tag).hasMatch()) {
                m_matchingTags[category->name()] += tag;
            }
        }
    }
}

bool DB::WildcardCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    Q_UNUSED(alreadyMatched)
    return eval(info);
}

bool DB::WildcardCategoryMatcher::eval(const DB::ImageInfoPtr info) const
{
    for (auto it = m_matchingTags.constKeyValueBegin(); it != m_matchingTags.constKeyValueEnd(); ++it) {
        const auto categoryName = (*it).first;
        const auto tags = (*it).second;
        if (info->hasCategoryInfo(categoryName, tags))
            return true;
    }
    return false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
