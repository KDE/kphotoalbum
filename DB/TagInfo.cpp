// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
//  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "TagInfo.h"

#include "Category.h"

namespace DB
{

TagInfo::TagInfo(const CategoryPtr &category, const QString &tag, QObject *parent)
    : QObject(parent)
    , m_category(category)
    , m_tag(tag)
{
    Q_ASSERT(category->items().contains(tag));
    connect(m_category.data(), &DB::Category::itemRenamed, this, &DB::TagInfo::updateTagName);
    connect(m_category.data(), &DB::Category::itemRemoved, this, &DB::TagInfo::removeTagName);
}

CategoryPtr TagInfo::category() const
{
    return m_category;
}

QString TagInfo::categoryName() const
{
    return m_category->name();
}

QString TagInfo::tagName() const
{
    return m_tag;
}

bool TagInfo::isValid() const
{
    return m_category && !m_tag.isNull();
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
