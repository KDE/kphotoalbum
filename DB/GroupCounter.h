/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GROUPCOUNTER_H
#define GROUPCOUNTER_H
#include "Category.h"

#include <Settings/SettingsData.h>

#include <QHash>

namespace DB
{
using Utilities::StringSet;

class GroupCounter
{
public:
    explicit GroupCounter(const QString &category);
    void count(const StringSet &, const ImageDate &date);
    QMap<QString, CountWithRange> result();

private:
    QHash<QString, QStringList> m_memberToGroup;
    QHash<QString, CountWithRange> m_groupCount;
};

}

#endif /* GROUPCOUNTER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
