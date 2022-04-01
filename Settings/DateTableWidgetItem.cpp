// SPDX-FileCopyrightText: 2015 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

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
