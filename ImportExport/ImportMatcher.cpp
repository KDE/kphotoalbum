/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportMatcher.h"

#include "ImportSettings.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <QGridLayout>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
using namespace ImportExport;

ImportMatcher::ImportMatcher(const QString &otherCategory, const QString &myCategory,
                             const QStringList &otherItems, const QStringList &myItems,
                             bool allowNew, QWidget *parent)
    : QScrollArea(parent)
    , m_otherCategory(otherCategory)
    , m_myCategory(myCategory)
{
    setWidgetResizable(true);
    QWidget *top = new QWidget(viewport());
    QVBoxLayout *layout = new QVBoxLayout(top);
    QWidget *grid = new QWidget;
    layout->addWidget(grid);
    layout->addStretch(1);

    QGridLayout *gridLay = new QGridLayout(grid);
    gridLay->setColumnStretch(1, 1);
    setWidget(top);

    QLabel *label = new QLabel(i18n("Key in file"), grid);
    label->setAutoFillBackground(true);
    label->setForegroundRole(QPalette::Dark);
    label->setBackgroundRole(QPalette::BrightText);
    label->setAlignment(Qt::AlignCenter);
    gridLay->addWidget(label, 0, 0);

    label = new QLabel(i18n("Key in your database"), grid);
    label->setAutoFillBackground(true);
    label->setForegroundRole(QPalette::Dark);
    label->setBackgroundRole(QPalette::BrightText);
    label->setAlignment(Qt::AlignCenter);
    gridLay->addWidget(label, 0, 1);

    int row = 1;
    for (QStringList::ConstIterator it = otherItems.begin(); it != otherItems.end(); ++it) {
        CategoryMatch *match = new CategoryMatch(allowNew, *it, myItems, grid, gridLay, row++);
        m_matchers.append(match);
    }
}

CategoryMatch::CategoryMatch(bool allowNew, const QString &kimFileItem, QStringList myItems, QWidget *parent, QGridLayout *grid, int row)
{
    m_checkbox = new QCheckBox(kimFileItem, parent);
    m_text = kimFileItem; // We can't just use QCheckBox::text() as Qt adds accelerators.
    m_checkbox->setChecked(true);
    grid->addWidget(m_checkbox, row, 0);

    m_combobox = new QComboBox;
    m_combobox->setEditable(allowNew);

    myItems.sort();
    m_combobox->addItems(myItems);
    QObject::connect(m_checkbox, &QCheckBox::toggled, m_combobox, &QComboBox::setEnabled);
    grid->addWidget(m_combobox, row, 1);

    if (myItems.contains(kimFileItem)) {
        m_combobox->setCurrentIndex(myItems.indexOf(kimFileItem));
    } else {
        // This item was not in my database
        QString match;
        for (QStringList::ConstIterator it = myItems.constBegin(); it != myItems.constEnd(); ++it) {
            if ((*it).contains(kimFileItem) || kimFileItem.contains(*it)) {
                // Either my item was a substring of the kim item or the other way around (Jesper is a substring of Jesper Pedersen)
                if (match.isEmpty())
                    match = *it;
                else {
                    match.clear();
                    break;
                }
            }
        }
        if (!match.isEmpty()) {
            // there was a single substring match
            m_combobox->setCurrentIndex(myItems.indexOf(match));
        } else {
            // Either none or multiple items matches
            if (allowNew) {
                m_combobox->addItem(kimFileItem);
                m_combobox->setCurrentIndex(m_combobox->count() - 1);
            } else
                m_checkbox->setChecked(false);
        }
    }
}

ImportExport::CategoryMatchSetting ImportExport::ImportMatcher::settings()
{
    CategoryMatchSetting res(m_myCategory, m_otherCategory);
    for (CategoryMatch *match : m_matchers) {
        if (match->m_checkbox->isChecked())
            res.add(match->m_combobox->currentText(), match->m_text);
    }
    return res;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ImportMatcher.cpp"
