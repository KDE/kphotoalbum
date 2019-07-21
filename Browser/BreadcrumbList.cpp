/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
