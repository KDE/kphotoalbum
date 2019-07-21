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

#ifndef REMOTECONTROL_IMAGENAMESTORE_H
#define REMOTECONTROL_IMAGENAMESTORE_H

#include <DB/FileName.h>

#include <QHash>
#include <QPair>

namespace RemoteControl
{

class ImageNameStore
{
public:
    ImageNameStore();
    DB::FileName operator[](int id);
    int operator[](const DB::FileName &fileName);
    int idForCategory(const QString &category, const QString &item);
    QPair<QString, QString> categoryForId(int id);

private:
    QHash<int, DB::FileName> m_idToNameMap;
    QHash<DB::FileName, int> m_nameToIdMap;
    QHash<QPair<QString, QString>, int> m_categoryToIdMap;
    QHash<int, QPair<QString, QString>> m_idToCategoryMap;
    int m_lastId = 0;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGENAMESTORE_H
