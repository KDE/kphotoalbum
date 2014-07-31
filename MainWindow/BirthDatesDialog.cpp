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
#include "BirthDatesDialog.h"
#include <KLocale>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"
#include "DB/Category.h"
#include <QPushButton>
#include <QScrollArea>
#include "CalendarPopup.h"
#include <QLineEdit>
#include <QShortcut>
#include "DirtyIndicator.h"

namespace MainWindow {

BirthDatesDialog::BirthDatesDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(i18n("Configure Birth Dates"));
    QVBoxLayout* vlay = new QVBoxLayout(this);

    QString txt = i18n("Set the date of birth for items (say people) here,\nand then see their age when viewing the images.");
    QLabel* label = new QLabel(txt);
    QFont fnt;
    fnt.setPixelSize(20);
    label->setFont(fnt);
    label->setAlignment(Qt::AlignHCenter);
    vlay->addWidget(label);
    vlay->addSpacing(20);

    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout(hlay);

    QLabel* categoryText = new QLabel(i18n("Current Category"));
    m_categoryBox = new QComboBox;

    m_filter = new QLineEdit;
    m_filter->setPlaceholderText(i18n("Filter (Alt+f)"));
    new QShortcut(Qt::AltModifier + Qt::Key_F, m_filter, SLOT(setFocus()));

    connect(m_filter,SIGNAL(textChanged(QString)), this, SLOT(resetCategory()));

    hlay->addWidget(categoryText);
    hlay->addWidget(m_categoryBox);
    hlay->addSpacing(100);
    hlay->addWidget(m_filter,1);

    m_area = new QScrollArea;
    m_area->setWidgetResizable(true);
    vlay->addWidget(m_area);

    QPushButton* close = new QPushButton(i18n("Close"));
    connect(close, SIGNAL(clicked()), this, SLOT(accept()));
    hlay = new QHBoxLayout;
    vlay->addLayout(hlay);

    hlay->addStretch(1);
    hlay->addWidget(close);

    int defaultIndex = 0;
    int index = 0;
    for (const DB::CategoryPtr& category: DB::ImageDB::instance()->categoryCollection()->categories()) {
        if(!category->isSpecialCategory()) {
            m_categoryBox->addItem(category->text(), category->name());
            if ( category->name() == QString::fromUtf8("Persons") || category->name() == QString::fromUtf8("People"))
                defaultIndex = index;
            ++index;
        }
    }

    connect(m_categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(changeCategory(int)));
    m_categoryBox->setCurrentIndex(defaultIndex);
    changeCategory(defaultIndex); // needed in case we didn't find the people category, to ensure the list is filled.
}

QSize BirthDatesDialog::sizeHint() const
{
    return QSize(800,800);
}

void BirthDatesDialog::changeCategory(int index)
{
    QWidget* widget = new QWidget;
    QGridLayout* grid = new QGridLayout(widget);
    grid->setColumnStretch(2,1);

    int row = 0;
    const QString categoryName = m_categoryBox->itemData(index).value<QString>();
    const DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(categoryName);
    QStringList items = category->items();
    items.sort();
    for (const QString& text : items) {
        if (!m_filter->text().isEmpty() && text.indexOf(m_filter->text(), 0, Qt::CaseInsensitive) == -1)
            continue;

        QLabel* label = new QLabel(text);
        const QDate date = category->birthDate(text);
        QPushButton* button = new QPushButton(textForDate(date));
        button->setProperty("date", date);
        button->setProperty("name", text);
        connect(button,SIGNAL(clicked()),this,SLOT(editDate()));
        grid->addWidget(label,row,0);
        grid->addWidget(button,row,1);
        ++row;
    }
    grid->setRowStretch(row,1);

    m_area->setWidget(widget);
    widget->show();
}

void BirthDatesDialog::resetCategory()
{
    changeCategory(m_categoryBox->currentIndex());
}

void BirthDatesDialog::editDate()
{
    if (!m_datePick) {
        m_datePick = new CalendarPopup;
        connect(m_datePick,SIGNAL(dateSelected(QDate)), this, SLOT(setDate(QDate)));
    }
    m_editButton = static_cast<QPushButton*>(sender());
    m_datePick->move(m_editButton->mapToGlobal(QPoint(0,0))-QPoint(m_datePick->sizeHint().width()/2, m_datePick->sizeHint().height()/2));
    m_datePick->setSelectedDate(m_editButton->property("date").value<QDate>());
    m_datePick->show();
}

void BirthDatesDialog::setDate(const QDate& date)
{
    m_datePick->hide();
    m_editButton->setText(textForDate(date));
    m_editButton->setProperty("date", date);

    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(m_categoryBox->itemData(m_categoryBox->currentIndex()).value<QString>());
    DirtyIndicator::markDirty();
    category->setBirthDate(m_editButton->property("name").value<QString>(), date);
}

QString BirthDatesDialog::textForDate(const QDate &date) const
{
    if (date.isNull())
        return QString::fromUtf8("---");
    else
        return KGlobal::locale()->formatDate(date, KLocale::ShortDate);
}

} // namespace Settings
