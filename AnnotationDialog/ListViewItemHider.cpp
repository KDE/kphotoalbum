// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Qt includes
#include <QRegularExpression>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

// Local includes
#include "ListSelect.h"
#include "ListViewItemHider.h"

#include <DB/ImageDB.h>

using namespace Utilities;

/**
 * \class AnnotationDialog::ListViewItemHider
 * \brief Helper class, used to hide/show listview items
 *
 * This is a helper class that is used to hide items in a listview. A leaf
 * will be hidden if then subclass implemented method \ref
 * shouldItemBeShown returns true for the given item. A parent node is
 * hidden if none of the children are shown, and \ref shouldItemBeShown
 * also returns false for the parent itself.
 */

/**
 * \class AnnotationDialog::ListViewTextMatchHider
 * \brief Helper class for showing items matching a given text string.
 */

/**
 * \class AnnotationDialog::ListViewCheckedHider
 * \brief Helper class for only showing items that are selected.
 */

bool AnnotationDialog::ListViewItemHider::setItemsVisible(QTreeWidgetItem *parentItem)
{
    bool anyChildrenVisible = false;
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *item = parentItem->child(i);
        bool anySubChildrenVisible = setItemsVisible(item);
        bool itemVisible = anySubChildrenVisible || shouldItemBeShown(item);
        item->setHidden(!itemVisible);
        anyChildrenVisible |= itemVisible;
    }
    return anyChildrenVisible;
}

AnnotationDialog::ListViewTextMatchHider::ListViewTextMatchHider(const QString &text, const AnnotationDialog::MatchType mt, QTreeWidget *listView)
    : m_text(text)
    , m_matchType(mt)
{
    setItemsVisible(listView->invisibleRootItem());
}

bool AnnotationDialog::ListViewTextMatchHider::shouldItemBeShown(QTreeWidgetItem *item)
{
    // Be sure not to display the "untagged image" tag if configured
    if (DB::ImageDB::instance()->untaggedCategoryFeatureConfigured()
        && !Settings::SettingsData::instance()->untaggedImagesTagVisible()) {
        const auto listSelect = dynamic_cast<ListSelect *>(item->treeWidget()->parent());
        Q_ASSERT(listSelect);
        if (Settings::SettingsData::instance()->untaggedCategory() == listSelect->category()) {
            if (item->text(0) == Settings::SettingsData::instance()->untaggedTag()) {
                return false;
            }
        }
    }

    switch (m_matchType) {
    case AnnotationDialog::MatchFromBeginning:
        return item->text(0).toLower().startsWith(m_text.toLower());
    case AnnotationDialog::MatchFromWordStart: {
        const QStringList itemWords = item->text(0).toLower().split(QRegularExpression(QStringLiteral("\\W+")), Qt::SkipEmptyParts);
        const QStringList searchWords = m_text.toLower().split(QRegularExpression(QStringLiteral("\\W+")), Qt::SkipEmptyParts);

        // all search words ...
        for (const auto &searchWord : searchWords) {
            bool found = false;
            // ... must match at least one word of the item
            for (const auto &itemWord : itemWords) {
                if (itemWord.startsWith(searchWord)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }
    case AnnotationDialog::MatchAnywhere:
        return item->text(0).toLower().contains(m_text.toLower());
    }
    // gcc believes this could be reached
    Q_ASSERT(false);
    return false;
}

bool AnnotationDialog::ListViewCheckedHider::shouldItemBeShown(QTreeWidgetItem *item)
{
    return item->checkState(0) != Qt::Unchecked;
}

AnnotationDialog::ListViewCheckedHider::ListViewCheckedHider(QTreeWidget *listView)
{
    setItemsVisible(listView->invisibleRootItem());
}

// vi:expandtab:tabstop=4 shiftwidth=4:
