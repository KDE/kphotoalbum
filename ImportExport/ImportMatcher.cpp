// SPDX-FileCopyrightText: 2003 - 2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImportMatcher.h"
#include "ImportSettings.h"
#include "Logging.h"

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

    QLabel *label = new QLabel(i18n("Key in import file"), grid);
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
    QObject::connect(m_combobox, &QComboBox::currentIndexChanged, m_combobox, [=, this](int index) {
        // A tooltip is set on the combo box if either a partial match or
        // no match is detected (see below).  Update the tooltip for the
        // new current index (exact matches do not have a tooltip).
        m_combobox->setToolTip(m_combobox->itemData(index, Qt::ToolTipRole).toString());
        });

    myItems.sort();
    m_combobox->addItems(myItems);
    QObject::connect(m_checkbox, &QCheckBox::toggled, m_combobox, &QComboBox::setEnabled);
    grid->addWidget(m_combobox, row, 1);

    if (myItems.contains(kimFileItem)) {
        qCDebug(ImportExportLog) << "Import item" << kimFileItem << "is already in the database";
        m_combobox->setCurrentIndex(myItems.indexOf(kimFileItem));
    } else {
        // This item is not in my database.
        QStringList partialMatches;
        for (QStringList::ConstIterator it = myItems.constBegin(); it != myItems.constEnd(); ++it) {
            if ((*it).contains(kimFileItem) || kimFileItem.contains(*it)) {
                // Either my item was a substring of the kim item or the other way around (Jesper is a substring of Jesper Pedersen)
                qCDebug(ImportExportLog) << "Partial match for import item" << kimFileItem << "to database item" << *it;
                partialMatches << *it;
            }
        }

        if (!partialMatches.isEmpty()) {
            // Select the first partial match.  The user has the option to change it.
            const auto currentIndex = myItems.indexOf(partialMatches.first());

            // There is at least one partial match with a key in the database, so mark
            // the matching items with an icon and a tooltip to inform the user.
            for (const QString &match : partialMatches) {
                const auto index = myItems.indexOf(match);

                m_combobox->setItemIcon(index, QIcon::fromTheme(QIcon::ThemeIcon::DialogInformation));

                const auto toolTip = i18n("The import key partially matches this key in your database");
                m_combobox->setItemData(index, toolTip, Qt::ToolTipRole);
            }

            // The lambda sets the tooltip on the combobox.
            m_combobox->setCurrentIndex(currentIndex);
        }

        if (allowNew) {
            // Append the import item to the combobox and mark it with an icon and
            // tooltip to inform the user that this item is not in the database
            // but they have the option to add it.
            qCDebug(ImportExportLog) << "Appending new import item" << kimFileItem;
            m_combobox->addItem(kimFileItem);

            const auto index = m_combobox->count() - 1;
            m_combobox->setItemIcon(index, QIcon::fromTheme(QIcon::ThemeIcon::DialogWarning));

            const auto toolTip = i18n("This key is not currently in your database");
            m_combobox->setItemData(index, toolTip, Qt::ToolTipRole);

            // If there is at least one partial match, the current index was set above.
            if (partialMatches.empty()) {
                // The lambda sets the tooltip on the combobox.
                m_combobox->setCurrentIndex(index);
            }
        } else {
            m_checkbox->setChecked(false);
            qCDebug(ImportExportLog) << "No match for import item" << kimFileItem;
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
