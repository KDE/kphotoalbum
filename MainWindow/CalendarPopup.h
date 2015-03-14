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

#ifndef SETTINGS_CALENDARPOPUP_H
#define SETTINGS_CALENDARPOPUP_H

// Qt includes
#include <QWidget>
#include <QDate>

// Qt classes
class QCalendarWidget;
class QLineEdit;

namespace MainWindow {

class CalendarPopup : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarPopup(QWidget* parent = 0);
    void setSelectedDate(const QDate& date);

signals:
    void dateSelected(const QDate& date = QDate());

protected slots:
    void parseDate(QString date);
    void checkDate();

private: // Functions
    void resetDate();

private: // variables
    QCalendarWidget* m_calendar;
    QString m_dateFormat;
    QLineEdit* m_dateInput;
};

} // namespace Settings

#endif // SETTINGS_CALENDARPOPUP_H
