/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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
