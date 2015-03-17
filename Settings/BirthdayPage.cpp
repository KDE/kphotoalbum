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

// Qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QShortcut>
#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QDebug>
#include <QFont>
#include <QCalendarWidget>

// KDE includes
#include <KPageWidgetItem>
#include <KLocale>

// Local includes
#include "BirthdayPage.h"
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"
#include "DB/Category.h"
#include "MainWindow/DirtyIndicator.h"
#include "DateTableWidgetItem.h"

Settings::BirthdayPage::BirthdayPage(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* dataLayout = new QHBoxLayout;
    mainLayout->addLayout(dataLayout);

    QVBoxLayout* itemsLayout = new QVBoxLayout;
    dataLayout->addLayout(itemsLayout);

    QHBoxLayout* itemsHeaderLayout = new QHBoxLayout;
    itemsLayout->addLayout(itemsHeaderLayout);

    QLabel* categoryText = new QLabel(i18n("Category:"));
    itemsHeaderLayout->addWidget(categoryText);

    m_categoryBox = new QComboBox;
    itemsHeaderLayout->addWidget(m_categoryBox);
    connect(m_categoryBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCategory(int)));

    m_filter = new QLineEdit;
    m_filter->setPlaceholderText(i18n("Filter (Alt+f)"));
    itemsHeaderLayout->addWidget(m_filter);
    connect(m_filter, SIGNAL(textChanged(QString)), this, SLOT(resetCategory()));
    new QShortcut(Qt::AltModifier + Qt::Key_F, m_filter, SLOT(setFocus()));

    m_dataView = new QTableWidget;
    m_dataView->setColumnCount(2);
    m_dataView->verticalHeader()->hide();
    m_dataView->setShowGrid(false);
    itemsLayout->addWidget(m_dataView);
    connect(m_dataView, SIGNAL(cellActivated(int,int)), this, SLOT(editDate(int,int)));

    QVBoxLayout* calendarLayout = new QVBoxLayout;
    dataLayout->addLayout(calendarLayout);

    calendarLayout->addStretch();

    m_birthdayOfLabel = new QLabel;
    calendarLayout->addWidget(m_birthdayOfLabel);

    m_dateInput = new QLineEdit;
    calendarLayout->addWidget(m_dateInput);
    connect(m_dateInput, SIGNAL(textEdited(QString)), this, SLOT(parseDate(QString)));
    connect(m_dateInput, SIGNAL(editingFinished()), this, SLOT(checkDate()));

    m_locale = KGlobal::locale();
    m_calendar = new QCalendarWidget;
    switch (m_locale->weekStartDay()) {
    case 1: m_calendar->setFirstDayOfWeek(Qt::Monday); break;
    case 2: m_calendar->setFirstDayOfWeek(Qt::Tuesday); break;
    case 3: m_calendar->setFirstDayOfWeek(Qt::Wednesday); break;
    case 4: m_calendar->setFirstDayOfWeek(Qt::Thursday); break;
    case 5: m_calendar->setFirstDayOfWeek(Qt::Friday); break;
    case 6: m_calendar->setFirstDayOfWeek(Qt::Saturday); break;
    case 7: m_calendar->setFirstDayOfWeek(Qt::Sunday); break;
    }
    calendarLayout->addWidget(m_calendar);
    connect(m_calendar, SIGNAL(clicked(QDate)), this, SLOT(setDate(QDate)));

    m_unsetButton = new QPushButton(i18n("unset"));
    calendarLayout->addWidget(m_unsetButton);
    connect(m_unsetButton, SIGNAL(clicked()), this, SLOT(removeDate()));

    calendarLayout->addStretch();

    QLabel* info = new QLabel(i18n("Set the date of birth for items (say people) here, "
                                   "and then see their age when viewing the images."));
    mainLayout->addWidget(info);

    m_noDateString = QString::fromUtf8("---");
    m_boldFont.setBold(true);

    disableCalendar();
}

void Settings::BirthdayPage::pageChange(KPageWidgetItem* page)
{
    if (page->widget() == this) {
        m_lastItem = nullptr;
        reload();
    }
}

void Settings::BirthdayPage::reload()
{
    m_dateInput->setText(QString());
    m_calendar->setSelectedDate(QDate::currentDate());

    disableCalendar();

    m_categoryBox->blockSignals(true);
    m_categoryBox->clear();

    int defaultIndex = 0;
    int index = 0;

    for (const DB::CategoryPtr& category: DB::ImageDB::instance()->categoryCollection()->categories()) {
        if (category->isSpecialCategory()) {
            continue;
        }
        m_categoryBox->addItem(category->text(), category->name());
        if (category->name() == QString::fromUtf8("People")) {
            defaultIndex = index;
        }
        ++index;
    }

    m_categoryBox->setCurrentIndex(defaultIndex);
    changeCategory(defaultIndex);

    m_categoryBox->blockSignals(false);
}

void Settings::BirthdayPage::resetCategory()
{
    changeCategory(m_categoryBox->currentIndex());
}

void Settings::BirthdayPage::changeCategory(int index)
{
    m_lastItem = nullptr;
    m_dataView->clear();
    m_dataView->setSortingEnabled(false);
    m_dataView->setHorizontalHeaderLabels(QStringList() << i18n("Name") << i18n("Birthday"));

    const QString categoryName = m_categoryBox->itemData(index).value<QString>();
    const DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(categoryName);
    QStringList items = category->items();

    m_dataView->setRowCount(items.count());
    int row = 0;

    for (const QString& text : items) {
        if (! m_filter->text().isEmpty()
            && text.indexOf(m_filter->text(), 0, Qt::CaseInsensitive) == -1) {
            continue;
        }

        QTableWidgetItem* nameItem = new QTableWidgetItem(text);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        m_dataView->setItem(row, 0, nameItem);

        QDate dateForItem;
        if (m_changedData.contains(categoryName)) {
            if (m_changedData[categoryName].contains(text)) {
                dateForItem = m_changedData[categoryName][text];
            } else {
                dateForItem = category->birthDate(text);
            }
        } else {
            dateForItem = category->birthDate(text);
        }

        DateTableWidgetItem* dateItem = new DateTableWidgetItem(textForDate(dateForItem));
        dateItem->setData(Qt::UserRole, dateForItem);
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        m_dataView->setItem(row, 1, dateItem);

        row++;
    }

    m_dataView->setSortingEnabled(true);
    m_dataView->sortItems(0);

    disableCalendar();
}

QString Settings::BirthdayPage::textForDate(const QDate& date) const
{
    if (date.isNull()) {
        return m_noDateString;
    } else {
        return KGlobal::locale()->formatDate(date, KLocale::ShortDate);
    }
}

void Settings::BirthdayPage::editDate(int row, int)
{
    m_dateInput->setEnabled(true);
    m_calendar->setEnabled(true);
    m_unsetButton->setEnabled(m_dataView->item(row, 1)->text() != m_noDateString);

    if (m_lastItem != nullptr) {
        m_lastItem->setFont(m_font);
        m_dataView->item(m_lastItem->row(), 1)->setFont(m_font);
    }

    m_dataView->item(row, 0)->setFont(m_boldFont);
    m_dataView->item(row, 1)->setFont(m_boldFont);

    m_birthdayOfLabel->setText(i18n("Birthday of %1:", m_dataView->item(row, 0)->text()));

    QString dateString = m_dataView->item(row, 1)->text();
    if (dateString != m_noDateString) {
        m_dateInput->setText(dateString);
        m_calendar->setSelectedDate(m_dataView->item(row, 1)->data(Qt::UserRole).toDate());
    } else {
        m_dateInput->setText(QString());
        m_dateInput->setPlaceholderText( i18n("Enter a date..."));
        m_calendar->setSelectedDate(QDate::currentDate());
    }

    m_lastItem = m_dataView->item(row, 0);
}

void Settings::BirthdayPage::parseDate(QString date)
{
    QDate parsedDate = m_locale->readDate(date);
    if (parsedDate != QDate()) {
        m_calendar->setSelectedDate(parsedDate);
        m_dateInput->setStyleSheet(QString());
    } else {
        m_dateInput->setStyleSheet(QString::fromUtf8("color:red;"));
    }
}

void Settings::BirthdayPage::checkDate()
{
    QDate parsedDate = m_locale->readDate(m_dateInput->text());
    if (parsedDate != QDate()) {
        setDate(parsedDate);
    }
}

void Settings::BirthdayPage::setDate(const QDate& date)
{
    const QString currentCategory = m_categoryBox->itemData(m_categoryBox->currentIndex()).value<QString>();
    if (! m_changedData.contains(currentCategory)) {
        m_changedData[currentCategory] = QMap<QString, QDate>();
    }

    const QString currentItem = m_dataView->item(m_dataView->currentRow(), 0)->text();
    m_changedData[currentCategory][currentItem] = date;

    m_dataView->item(m_dataView->currentRow(), 1)->setText(textForDate(date));
    m_dataView->item(m_dataView->currentRow(), 1)->setData(Qt::UserRole, date);

    m_unsetButton->setEnabled(true);
}

void Settings::BirthdayPage::disableCalendar()
{
    m_dateInput->setEnabled(false);
    m_calendar->setEnabled(false);
    m_unsetButton->setEnabled(false);
    m_birthdayOfLabel->setText(i18n("<i>Select an item on the left to edit the birthday</i>"));
}

void Settings::BirthdayPage::discardChanges()
{
    m_changedData.clear();
}

void Settings::BirthdayPage::saveSettings()
{
    QMapIterator<QString, QMap<QString, QDate>> changedCategory(m_changedData);
    while (changedCategory.hasNext()) {
        changedCategory.next();
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()
                                                          ->categoryForName(changedCategory.key());

        QMapIterator<QString, QDate> changedItem(changedCategory.value());
        while (changedItem.hasNext()) {
            changedItem.next();
            category->setBirthDate(changedItem.key(), changedItem.value());
        }
    }

    if (m_changedData.size() > 0) {
        MainWindow::DirtyIndicator::markDirty();
        m_changedData.clear();
    }
}

void Settings::BirthdayPage::removeDate()
{
    m_dateInput->setText(QString());
    m_calendar->setSelectedDate(QDate::currentDate());
    setDate(QDate());
}
