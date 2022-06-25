/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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

#include "moc_ImageDetails.cpp"
