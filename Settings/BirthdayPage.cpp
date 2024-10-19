// SPDX-FileCopyrightText: 2014-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Qt includes
#include <QCalendarWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QShortcut>
#include <QTableWidget>
#include <QVBoxLayout>

// KDE includes
#include <KColorScheme>
#include <KLocalizedString>
#include <KPageWidgetModel>

// Local includes
#include "BirthdayPage.h"
#include "DateTableWidgetItem.h"

#include <DB/Category.h>
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <MainWindow/DirtyIndicator.h>

Settings::BirthdayPage::BirthdayPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *dataLayout = new QHBoxLayout;
    mainLayout->addLayout(dataLayout);

    QVBoxLayout *itemsLayout = new QVBoxLayout;
    dataLayout->addLayout(itemsLayout);

    QHBoxLayout *itemsHeaderLayout = new QHBoxLayout;
    itemsLayout->addLayout(itemsHeaderLayout);

    QLabel *categoryText = new QLabel(i18n("Category:"));
    itemsHeaderLayout->addWidget(categoryText);

    m_categoryBox = new QComboBox;
    itemsHeaderLayout->addWidget(m_categoryBox);
    connect(m_categoryBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &BirthdayPage::changeCategory);

    m_filter = new QLineEdit;
    itemsHeaderLayout->addWidget(m_filter);
    connect(m_filter, &QLineEdit::textChanged, this, &BirthdayPage::resetCategory);
    const auto filterShortcut = new QShortcut(QKeySequence(Qt::AltModifier | Qt::Key_F), m_filter, SLOT(setFocus()));
    m_filter->setPlaceholderText(i18nc("@label:textedit", "Filter ... (%1)", filterShortcut->key().toString(QKeySequence::NativeText)));

    if (QLocale().dateFormat(QLocale::ShortFormat).contains(QString::fromUtf8("yyyy"))) {
        m_dateFormats << QLocale().dateFormat(QLocale::ShortFormat);
    } else {
        m_dateFormats << QLocale().dateFormat(QLocale::ShortFormat).replace(QString::fromUtf8("yy"), QString::fromUtf8("yyyy"));
    }
    m_dateFormats << QLocale().dateFormat(QLocale::ShortFormat)
                  << QLocale().dateFormat(QLocale::LongFormat);

    m_dataView = new QTableWidget;
    m_dataView->setColumnCount(2);
    m_dataView->verticalHeader()->hide();
    m_dataView->setShowGrid(false);
    itemsLayout->addWidget(m_dataView);
    connect(m_dataView, &QTableWidget::cellClicked, this, &BirthdayPage::editDate);

    QVBoxLayout *calendarLayout = new QVBoxLayout;
    dataLayout->addLayout(calendarLayout);

    calendarLayout->addStretch();

    m_birthdayOfLabel = new QLabel;
    calendarLayout->addWidget(m_birthdayOfLabel);

    m_dateInput = new QLineEdit;
    calendarLayout->addWidget(m_dateInput);
    connect(m_dateInput, &QLineEdit::textEdited, this, &BirthdayPage::checkDateInput);
    connect(m_dateInput, &QLineEdit::editingFinished, this, &BirthdayPage::checkDate);

    m_calendar = new QCalendarWidget;
    calendarLayout->addWidget(m_calendar);
    connect(m_calendar, &QCalendarWidget::clicked, this, &BirthdayPage::setDate);

    m_unsetButton = new QPushButton(i18n("Remove birthday"));
    calendarLayout->addWidget(m_unsetButton);
    connect(m_unsetButton, &QPushButton::clicked, this, &BirthdayPage::removeDate);

    calendarLayout->addStretch();

    QLabel *info = new QLabel(i18n("Set the date of birth for items (say people) here, "
                                   "and then see their age when viewing the images."));
    mainLayout->addWidget(info);

    m_noDateString = QString::fromUtf8("---");
    m_boldFont.setBold(true);

    disableCalendar();
}

void Settings::BirthdayPage::pageChange(KPageWidgetItem *page)
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

    for (const DB::CategoryPtr &category : DB::ImageDB::instance()->categoryCollection()->categories()) {
        if (category->isSpecialCategory()) {
            continue;
        }
        m_categoryBox->addItem(category->name());
        if (category->name() == i18n("People")) {
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

    const QString categoryName = m_categoryBox->itemText(index);
    const DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(categoryName);
    QStringList items = category->items();

    m_dataView->setRowCount(items.count());
    int row = 0;

    for (const QString &text : items) {
        if (!m_filter->text().isEmpty()
            && text.indexOf(m_filter->text(), 0, Qt::CaseInsensitive) == -1) {
            m_dataView->setRowCount(m_dataView->rowCount() - 1);
            continue;
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(text);
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

        DateTableWidgetItem *dateItem = new DateTableWidgetItem(textForDate(dateForItem));
        dateItem->setData(Qt::UserRole, dateForItem);
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        m_dataView->setItem(row, 1, dateItem);

        row++;
    }

    m_dataView->setSortingEnabled(true);
    m_dataView->sortItems(0);

    disableCalendar();
}

QString Settings::BirthdayPage::textForDate(const QDate &date) const
{
    if (date.isNull()) {
        return m_noDateString;
    } else {
        return QLocale().toString(date, m_dateFormats.at(0));
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
        m_dateInput->setPlaceholderText(i18n("Enter a date..."));
        m_calendar->setSelectedDate(QDate::currentDate());
    }

    m_lastItem = m_dataView->item(row, 0);
}

QDate Settings::BirthdayPage::parseDate(QString date)
{
    QDate parsedDate = QDate();
    for (const QString &format : m_dateFormats) {
        parsedDate = QDate::fromString(date, format);
        if (parsedDate.isValid()) {
            return parsedDate;
        }
    }
    return parsedDate;
}

void Settings::BirthdayPage::checkDateInput(QString date)
{
    QDate parsedDate = parseDate(date);
    if (parsedDate.isValid() || date.isEmpty()) {
        m_calendar->setSelectedDate(parsedDate);
        m_dateInput->setPalette(palette());
        // re-enable palette propagation:
        m_dateInput->setAttribute(Qt::WA_SetPalette);
    } else {
        auto errorPalette = m_dateInput->palette();
        KColorScheme::adjustForeground(errorPalette, KColorScheme::ForegroundRole::NegativeText, QPalette::Text);
        KColorScheme::adjustBackground(errorPalette, KColorScheme::BackgroundRole::NegativeBackground, QPalette::Base);
        m_dateInput->setPalette(errorPalette);
    }
}

void Settings::BirthdayPage::checkDate()
{
    QDate parsedDate = parseDate(m_dateInput->text());
    if (parsedDate.isValid()) {
        setDate(parsedDate);
    }
}

void Settings::BirthdayPage::setDate(const QDate &date)
{
    const QString currentCategory = m_categoryBox->currentText();
    if (!m_changedData.contains(currentCategory)) {
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
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(changedCategory.key());

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

#include "moc_BirthdayPage.cpp"
