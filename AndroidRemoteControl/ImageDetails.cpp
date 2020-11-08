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

#include "ImageDetails.h"

namespace RemoteControl
{

ImageDetails &ImageDetails::instance()
{
    static ImageDetails instance;
    return instance;
}

QStringList ImageDetails::categories() const
{
    return m_categories.keys();
}

QStringList ImageDetails::itemsOfCategory(const QString &category)
{
    auto list = m_categories[category];
    QStringList res;
    std::transform(list.begin(), list.end(), std::back_inserter(res),
                   [](CategoryItemDetails &item) { return item.name; });
    return res;
}

void ImageDetails::clear()
{
    m_fileName.clear();
    m_date.clear();
    m_description.clear();
    m_categories.clear();
    emit updated();
}

void ImageDetails::setData(const ImageDetailsResult &data)
{
    m_fileName = data.fileName;
    m_date = data.date;
    m_description = data.description;
    m_categories = data.categories;
    emit updated();
}

QString ImageDetails::age(const QString &category, const QString &item)
{
    auto list = m_categories[category];
    auto res = std::find_if(list.begin(), list.end(),
                            [&category, &item](const CategoryItemDetails &candidate) {
                                return candidate.name == item;
                            });
    return (*res).age;
}
} // namespace RemoteControl
