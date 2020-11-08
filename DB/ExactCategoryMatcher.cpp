/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ExactCategoryMatcher.h"

#include "ImageInfo.h"
#include "Logging.h"

DB::ExactCategoryMatcher::ExactCategoryMatcher(const QString category)
    : m_category(category)
    , m_matcher(nullptr)
{
}

DB::ExactCategoryMatcher::~ExactCategoryMatcher()
{
    if (m_matcher) {
        delete m_matcher;
        m_matcher = nullptr;
    }
}

void DB::ExactCategoryMatcher::setMatcher(CategoryMatcher *subMatcher)
{
    m_matcher = subMatcher;
    if (m_matcher)
        // always collect matched tags of _matcher:
        m_matcher->setShouldCreateMatchedSet(true);
}

bool DB::ExactCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet> &alreadyMatched)
{
    // it makes no sense to put one ExactCategoryMatcher into another, so we ignore alreadyMatched.
    Q_UNUSED(alreadyMatched);

    if (!m_matcher)
        return false;

    QMap<QString, StringSet> matchedTags;

    // first, do a regular match and collect all matched Tags.
    if (!m_matcher->eval(info, matchedTags))
        return false;

    // if the match succeeded, check if it is exact:
    for (const QString &item : info->itemsOfCategory(m_category))
        if (!matchedTags[m_category].contains(item))
            return false; // tag was not contained in matcher
    return true;
}

void DB::ExactCategoryMatcher::debug(int level) const
{
    qCDebug(DBCategoryMatcherLog, "%sEXACT:", qPrintable(spaces(level)));
    m_matcher->debug(level + 1);
}

void DB::ExactCategoryMatcher::setShouldCreateMatchedSet(bool)
{
    // no-op:
    // shouldCreateMatchedSet is already set to true for _matcher;
    // setting this to false would disable the ExactCategoryMatcher, so it is ignored.

    // only ExactCategoryMatcher ever calls setShouldCreateMatchedSet.
    // ExactCategoryMatcher are never stacked, so this can't be called.
    Q_ASSERT(false);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
