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
