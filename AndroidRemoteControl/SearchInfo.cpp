/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SearchInfo.h"

namespace RemoteControl
{

void RemoteControl::SearchInfo::addCategory(const QString &category)
{
    m_categories.append(category);
}

void SearchInfo::addValue(const QString &value)
{
    m_values.append(value);
}

void SearchInfo::pop()
{
    m_categories.pop();
    if (m_values.count() > m_categories.count())
        m_values.pop();
}

void SearchInfo::clear()
{
    m_categories.clear();
    m_values.clear();
}

QString SearchInfo::currentCategory() const
{
    if (m_categories.isEmpty())
        return {};
    return m_categories.top();
}

QDataStream &operator<<(QDataStream &stream, const SearchInfo &searchInfo)
{
    stream << searchInfo.m_categories << searchInfo.m_values;
    return stream;
}

QDataStream &operator>>(QDataStream &stream, SearchInfo &searchInfo)
{
    stream >> searchInfo.m_categories >> searchInfo.m_values;
    return stream;
}

QList<std::tuple<QString, QString>> RemoteControl::SearchInfo::values() const
{
    QList<std::tuple<QString, QString>> result;
    for (int i = 0; i < m_values.count(); ++i)
        result.append(std::make_tuple(m_categories[i], m_values[i]));
    return result;
}

} // namespace RemoteControl
