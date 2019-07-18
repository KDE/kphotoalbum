/* Copyright (C) 2015 Tobias Leupold <tobias.leupold@web.de>

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

// Qt includes
#include <QDate>

// Local includes
#include "DateTableWidgetItem.h"

Settings::DateTableWidgetItem::DateTableWidgetItem(const QString &text)
{
    setText(text);
}

bool Settings::DateTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    if (data(Qt::UserRole).toDate() == QDate()
        && other.data(Qt::UserRole).toDate() != QDate()) {
        return false;
    } else if (data(Qt::UserRole).toDate() != QDate()
               && other.data(Qt::UserRole).toDate() == QDate()) {
        return true;
    } else {
        return data(Qt::UserRole).toDate() < other.data(Qt::UserRole).toDate();
    }
}
