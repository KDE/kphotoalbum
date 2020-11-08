/* SPDX-FileCopyrightText: 2014-2015 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
