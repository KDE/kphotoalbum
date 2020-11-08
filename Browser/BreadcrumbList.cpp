/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "BreadcrumbList.h"

#include <QStringList>

Browser::BreadcrumbList Browser::BreadcrumbList::latest() const
{
    BreadcrumbList result;
    for (int i = count() - 1; i >= 0; --i) {
        const Breadcrumb crumb = at(i);
        const QString txt = crumb.text();
        if (!txt.isEmpty() || crumb.isView())
            result.prepend(crumb);

        if (crumb.isBeginning())
            break;
    }

    return result;
}

QString Browser::BreadcrumbList::toString() const
{
    QStringList list;
    for (const Breadcrumb &item : latest())
        list.append(item.text());

    return list.join(QString::fromLatin1(" > "));
}
// vi:expandtab:tabstop=4 shiftwidth=4:
