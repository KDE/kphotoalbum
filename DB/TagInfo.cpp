// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
//  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "TagInfo.h"

#include "Category.h"

namespace DB
{

TagInfo::TagInfo()
    : QObject()
    , m_category(nullptr)
    , m_tag()
{
}

TagInfo::TagInfo(Category *category, const QString &tag)
    : QObject(category)
    , m_category(category)
    , m_tag(tag)
{
    Q_ASSERT(category->items().contains(tag));
    connect(m_category, &DB::Category::itemRenamed, this, &DB::TagInfo::updateTagName);
    connect(m_category, &DB::Category::itemRemoved, this, &DB::TagInfo::removeTagName);
}

Category *TagInfo::category() const
{
    return m_category;
}

QString TagInfo::categoryName() const
{
    return (m_category) ? m_category->name() : QString();
}

QString TagInfo::tagName() const
{
    return m_tag;
}

bool TagInfo::isValid() const
{
    return m_category != nullptr && !m_tag.isNull();
}

bool TagInfo::isNull() const
{
    return m_category == nullptr && m_tag.isNull();
}

void TagInfo::updateTagName(const QString &oldName, const QString &newName)
{
    if (oldName == m_tag) {
        m_tag = newName;
    }
}

void TagInfo::removeTagName(const QString &name)
{
    if (name == m_tag) {
        m_tag.clear();
    }
}

} // namespace DB
