/* Copyright (C) 2014-2015 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef BIRTHDAYPAGE_H
#define BIRTHDAYPAGE_H

// Qt includes
#include <QDate>
#include <QMap>
#include <QWidget>

// Qt classes
class QLineEdit;
class QComboBox;
class QTableWidget;
class QFont;
class QCalendarWidget;
class QPushButton;
class QTableWidgetItem;
class QLabel;

// KDE classes
class KPageWidgetItem;

namespace Settings
{

class BirthdayPage : public QWidget
{
    Q_OBJECT

public slots:
    void pageChange(KPageWidgetItem *page);
    void discardChanges();
    void saveSettings();

private slots:
    void changeCategory(int index);
    void resetCategory();
    void editDate(int row, int);
    QDate parseDate(QString date);
    void checkDateInput(QString date);
    void checkDate();
    void setDate(const QDate &date);
    void removeDate();

public:
    BirthdayPage(QWidget *parent);
    void reload();

private: // Functions
    QString textForDate(const QDate &date) const;
    void disableCalendar();

private: // Variables
    QLineEdit *m_filter;
    QComboBox *m_categoryBox;
    QTableWidget *m_dataView;
    QTableWidgetItem *m_lastItem = nullptr;
    QFont m_font;
    QFont m_boldFont;
    QLineEdit *m_dateInput;
    QCalendarWidget *m_calendar;
    QPushButton *m_unsetButton;
    QString m_noDateString;
    QMap<QString, QMap<QString, QDate>> m_changedData;
    QLabel *m_birthdayOfLabel;
    QStringList m_dateFormats;
};

}

#endif // BIRTHDAYPAGE_H
