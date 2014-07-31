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
#include "CalendarPopup.h"

#include <QCalendarWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <KLocale>

namespace MainWindow {

CalendarPopup::CalendarPopup(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(Qt::Popup);

    QVBoxLayout* vlay = new QVBoxLayout(this);

    m_calendar = new QCalendarWidget;
    vlay->addWidget(m_calendar);

    QPushButton* button = new QPushButton(i18n("unset"));
    vlay->addWidget(button);

    connect(m_calendar, SIGNAL(clicked(QDate)), this, SIGNAL(dateSelected(QDate)));
    connect(button, SIGNAL(clicked()), this, SIGNAL(dateSelected()));
}

void CalendarPopup::setSelectedDate(const QDate& date)
{
    m_calendar->setSelectedDate(date);
}

} // namespace Settings
