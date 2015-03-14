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

// Qt includes
#include <QCalendarWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>

// KDE includes
#include <KLocale>
#include <KGlobal>

// Local includes
#include "CalendarPopup.h"

namespace MainWindow {

CalendarPopup::CalendarPopup(QWidget* parent) : QWidget(parent)
{
    setWindowFlags(Qt::Popup);

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Line edit to enter the date directly
    m_dateInput = new QLineEdit;
    layout->addWidget(m_dateInput);
    connect(m_dateInput, SIGNAL(textEdited(QString)), this, SLOT(parseDate(QString)));
    connect(m_dateInput, SIGNAL(editingFinished()), this, SLOT(checkDate()));

    // Try to assemble a parsing mask from the local short date format
    m_dateFormat = KGlobal::locale()->dateFormatShort();
    m_dateFormat.replace(QString::fromUtf8("%e"), QString::fromUtf8("d"));
    m_dateFormat.replace(QString::fromUtf8("%d"), QString::fromUtf8("dd"));
    m_dateFormat.replace(QString::fromUtf8("%n"), QString::fromUtf8("M"));
    m_dateFormat.replace(QString::fromUtf8("%m"), QString::fromUtf8("MM"));
    m_dateFormat.replace(QString::fromUtf8("%b"), QString::fromUtf8("MMM"));
    m_dateFormat.replace(QString::fromUtf8("%B"), QString::fromUtf8("MMMM"));
    m_dateFormat.replace(QString::fromUtf8("%y"), QString::fromUtf8("yy"));
    m_dateFormat.replace(QString::fromUtf8("%Y"), QString::fromUtf8("yyyy"));

    // Add the calendar widget and set the user's chosen first day of the week
    m_calendar = new QCalendarWidget;
    switch (KGlobal::locale()->weekStartDay()) {
    case 1: m_calendar->setFirstDayOfWeek(Qt::Monday); break;
    case 2: m_calendar->setFirstDayOfWeek(Qt::Tuesday); break;
    case 3: m_calendar->setFirstDayOfWeek(Qt::Wednesday); break;
    case 4: m_calendar->setFirstDayOfWeek(Qt::Thursday); break;
    case 5: m_calendar->setFirstDayOfWeek(Qt::Friday); break;
    case 6: m_calendar->setFirstDayOfWeek(Qt::Saturday); break;
    case 7: m_calendar->setFirstDayOfWeek(Qt::Sunday); break;
    }
    layout->addWidget(m_calendar);
    connect(m_calendar, SIGNAL(clicked(QDate)), this, SIGNAL(dateSelected(QDate)));

    QPushButton* unsetButton = new QPushButton(i18n("unset"));
    layout->addWidget(unsetButton);
    connect(unsetButton, SIGNAL(clicked()), this, SIGNAL(dateSelected()));
}

void CalendarPopup::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    m_dateInput->setFocus();
}

void CalendarPopup::setSelectedDate(const QDate& date)
{
    m_calendar->setSelectedDate(date);
}

void CalendarPopup::parseDate(QString date)
{
    QDate parsedDate = QDate::fromString(date, m_dateFormat);
    if (parsedDate != QDate()) {
        m_calendar->setSelectedDate(parsedDate);
        m_dateInput->setStyleSheet(QString());
    } else {
        m_dateInput->setStyleSheet(QString::fromUtf8("color:red;"));
    }
}

void CalendarPopup::checkDate()
{
    QDate parsedDate = QDate::fromString(m_dateInput->text(), m_dateFormat);
    if (parsedDate != QDate()) {
        emit dateSelected(parsedDate);
    }
}

} // namespace Settings
