// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CategoryCollection.h"

using namespace DB;

void CategoryCollection::slotItemRenamed(const QString &oldName, const QString &newName)
{
    Q_EMIT itemRenamed(static_cast<Category *>(const_cast<QObject *>(sender())), oldName, newName);
}

void CategoryCollection::slotItemRemoved(const QString &item)
{
    Q_EMIT itemRemoved(static_cast<Category *>(const_cast<QObject *>(sender())), item);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_CategoryCollection.cpp"
