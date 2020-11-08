/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageNameStore.h"

#include <DB/ImageDB.h>

namespace RemoteControl
{
ImageNameStore::ImageNameStore()
{
    // To avoid delays when the user shows all images the first time, lets pull all images now.
    for (const DB::FileName &fileName : DB::ImageDB::instance()->files()) {
        m_lastId++;
        m_idToNameMap.insert(m_lastId, fileName);
        m_nameToIdMap.insert(fileName, m_lastId);
    }
}

DB::FileName ImageNameStore::operator[](int id)
{
    return m_idToNameMap[id];
}

int ImageNameStore::operator[](const DB::FileName &fileName)
{
    auto iterator = m_nameToIdMap.find(fileName);
    if (iterator == m_nameToIdMap.end()) {
        m_lastId++;
        m_nameToIdMap.insert(fileName, m_lastId);
        m_idToNameMap.insert(m_lastId, fileName);
        return m_lastId;
    }
    return *iterator;
}

int ImageNameStore::idForCategory(const QString &category, const QString &item)
{
    auto key = qMakePair(category, item);
    auto it = m_categoryToIdMap.find(key);
    if (it == m_categoryToIdMap.end()) {
        m_lastId++;
        m_categoryToIdMap.insert(key, m_lastId);
        m_idToCategoryMap.insert(m_lastId, key);
        return m_lastId;
    } else
        return *it;
}

QPair<QString, QString> ImageNameStore::categoryForId(int id)
{
    return m_idToCategoryMap[id];
}

} // namespace RemoteControl
