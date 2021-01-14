/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECONTROL_IMAGENAMESTORE_H
#define REMOTECONTROL_IMAGENAMESTORE_H

#include <kpabase/FileName.h>

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
