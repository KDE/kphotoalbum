/* Copyright (C) 2003-2015 Jesper K. Pedersen <blackie@kde.org>

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
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

// Local includes
#include "ListSelect.h"
#include "ListViewItemHider.h"

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
    if (Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured()
        && !Settings::SettingsData::instance()->untaggedImagesTagVisible()) {
        if (Settings::SettingsData::instance()->untaggedCategory()
            == dynamic_cast<ListSelect *>(item->treeWidget()->parent())->category()) {
            if (item->text(0) == Settings::SettingsData::instance()->untaggedTag()) {
                return false;
            }
        }
    }

    switch (m_matchType) {
    case AnnotationDialog::MatchFromBeginning:
        return item->text(0).toLower().startsWith(m_text.toLower());
    case AnnotationDialog::MatchFromWordStart: {
        QStringList itemWords = item->text(0).toLower().split(QRegExp(QString::fromUtf8("\\W+")),
                                                              QString::SkipEmptyParts);
        QStringList searchWords = m_text.toLower().split(QRegExp(QString::fromUtf8("\\W+")),
                                                         QString::SkipEmptyParts);

        // all search words ...
        Q_FOREACH (const auto searchWord, searchWords) {
            bool found = false;
            // ... must match at least one word of the item
            Q_FOREACH (const auto itemWord, itemWords) {
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
