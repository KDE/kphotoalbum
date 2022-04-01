// SPDX-FileCopyrightText: 2015 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef DATETABLEWIDGETITEM_H
#define DATETABLEWIDGETITEM_H

// Qt includes
#include <QTableWidgetItem>

namespace Settings
{

class DateTableWidgetItem : public QTableWidgetItem
{

public:
    DateTableWidgetItem(const QString &text);
    bool operator<(const QTableWidgetItem &other) const override;

};

}

#endif // DATETABLEWIDGETITEM
