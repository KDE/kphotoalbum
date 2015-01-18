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
#ifndef SETTINGS_BIRTHDATESDIALOG_H
#define SETTINGS_BIRTHDATESDIALOG_H

#include <QDialog>
class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QScrollArea;
class QDate;
class QLineEdit;

namespace MainWindow {
class CalendarPopup;

class BirthDatesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BirthDatesDialog(QWidget *parent = 0);
    QSize sizeHint() const;

private slots:
    void changeCategory(int index);
    void resetCategory();
    void editDate();
    void setDate(const QDate&);

private:
    QString textForDate(const QDate& date) const;
    QComboBox* m_categoryBox;
    QScrollArea* m_area;
    CalendarPopup* m_datePick = nullptr;
    QPushButton* m_editButton;
    QLineEdit* m_filter;
};

} // namespace Settings

#endif // SETTINGS_BIRTHDATESDIALOG_H
