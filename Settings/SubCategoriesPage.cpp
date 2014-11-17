/* Copyright (C) 2003-2014 Jesper K. Pedersen <blackie@kde.org>

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
#include <QListWidget>
#include <QLabel>
#include <QDebug>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QFont>
#include <QAction>
#include <QMenu>
#include <QGridLayout>

// KDE includes
#include <KMessageBox>
#include <KInputDialog>
#include <KLocale>
#include <KStringListValidator>

// Local includes
#include "DB/CategoryCollection.h"
#include "SubCategoriesPage.h"
#include "CategoryTree.h"

Settings::SubCategoriesPage::SubCategoriesPage(QWidget* parent) : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    // The category and group tree
    layout->addWidget(new QLabel(i18n("Categories and groups:")), 0, 0);
    m_categoryTreeWidget = new CategoryTree(this);
    m_categoryTreeWidget->header()->hide();
    m_categoryTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_categoryTreeWidget, 1, 0);
    connect(m_categoryTreeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showTreeContextMenu(QPoint)));
    connect(m_categoryTreeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotGroupSelected(QTreeWidgetItem*)));

    // The member list
    m_selectGroupToAddTags = i18n("<i>Select a group on the left side to add tags to it</i>");
    m_tagsInGroupLabel = new QLabel(m_selectGroupToAddTags);
    layout->addWidget(m_tagsInGroupLabel, 0, 1);
    m_membersListWidget = new QListWidget;
    m_membersListWidget->setEnabled(false);
    m_membersListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_membersListWidget, 1, 1);
    connect(m_membersListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(checkItemSelection(QListWidgetItem*)));
    connect(m_membersListWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showMembersContextMenu(QPoint)));

    // Context menu actions
    m_newGroupAction = new QAction(i18n("Add group ..."), this);
    connect(m_newGroupAction, SIGNAL(triggered()), this, SLOT(slotAddGroup()));
    m_renameAction = new QAction(this);
    connect(m_renameAction, SIGNAL(triggered()), this, SLOT(slotRenameGroup()));
    m_deleteAction = new QAction(this);
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteGroup()));
    m_deleteMemberAction = new QAction(this);
    connect(m_deleteMemberAction, SIGNAL(triggered()), this, SLOT(slotDeleteMember()));
    m_renameMemberAction = new QAction(this);
    connect(m_renameMemberAction, SIGNAL(triggered()), this, SLOT(slotRenameMember()));

    m_memberMap = DB::ImageDB::instance()->memberMap();

    connect(DB::ImageDB::instance()->categoryCollection(),
            SIGNAL(itemRemoved(DB::Category*,QString)),
            &m_memberMap, SLOT(deleteItem(DB::Category*,QString)));

    connect(DB::ImageDB::instance()->categoryCollection(),
            SIGNAL(itemRenamed(DB::Category*,QString,QString)),
            &m_memberMap, SLOT(renameItem(DB::Category*,QString,QString)));
}

void Settings::SubCategoriesPage::updateCategoryTree()
{
    // Store all expanded items so that they can be expanded after reload
    QList<QPair<QString, QString>> expandedItems = QList<QPair<QString, QString>>();
    QTreeWidgetItemIterator it(m_categoryTreeWidget);
    while (*it) {
        if ((*it)->isExpanded()) {
            if ((*it)->parent() == nullptr) {
                expandedItems.append(QPair<QString, QString>((*it)->text(0), QString()));
            } else {
                expandedItems.append(
                    QPair<QString, QString>((*it)->text(0), (*it)->parent()->text(0))
                );
            }
        }
        ++it;
    }

    m_categoryTreeWidget->clear();

    // Create a tree view of all groups and their sub-groups

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for(QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        if ((*it)->isSpecialCategory()) {
            continue;
        }

        // Add the real categories as top-level items
        QTreeWidgetItem* topLevelItem = new QTreeWidgetItem;
        topLevelItem->setText(0, (*it)->text());
        topLevelItem->setFlags(topLevelItem->flags() & Qt::ItemIsEnabled);
        QFont font = topLevelItem->font(0);
        font.setWeight(QFont::Bold);
        topLevelItem->setFont(0, font);
        m_categoryTreeWidget->addTopLevelItem(topLevelItem);

        // Build a map with all members for each group
        QMap<QString, QStringList> membersForGroup;
        QStringList allGroups = m_memberMap.groups((*it)->name());
        foreach (const QString group, allGroups) {
            // FIXME: Why does the member map return an empty category?!
            if (group == QString()) {
                continue;
            }

            QStringList allMembers = m_memberMap.members((*it)->name(), group, false);
            foreach (const QString member, allMembers) {
                membersForGroup[group] << member;
            }

            // We add an empty member placeholder if the group currently has no members.
            // Otherwise, it won't be added.
            if (! membersForGroup.contains(group)) {
                membersForGroup[group] == QStringList();
            }
        }

        // Add all groups (their sub-groups will be added recursively)
        addSubCategories(topLevelItem, membersForGroup, allGroups);
    }

    // Order the items alphabetically
    m_categoryTreeWidget->sortItems(0, Qt::AscendingOrder);

    // Re-expand all previously expanded items
    QTreeWidgetItemIterator it2(m_categoryTreeWidget);
    while (*it2) {
        if ((*it2)->parent() == nullptr) {
            if (expandedItems.contains(QPair<QString, QString>((*it2)->text(0), QString()))) {
                (*it2)->setExpanded(true);
            }
        } else {
            if (expandedItems.contains(QPair<QString, QString>((*it2)->text(0), (*it2)->parent()->text(0)))) {
                (*it2)->setExpanded(true);
            }
        }
        ++it2;
    }
}

void Settings::SubCategoriesPage::addSubCategories(QTreeWidgetItem* superCategory,
                                                   QMap<QString, QStringList>& membersForGroup,
                                                   QStringList& allGroups)
{
    // Process all group members
    QMap<QString, QStringList>::iterator memIt1;
    for (memIt1 = membersForGroup.begin(); memIt1 != membersForGroup.end(); ++memIt1) {
        bool isSubGroup = false;

        // Search for a membership in another group for the current group
        QMap<QString, QStringList>::iterator memIt2;
        for (memIt2 = membersForGroup.begin(); memIt2 != membersForGroup.end(); ++memIt2) {
            if (memIt2.value().contains(memIt1.key())) {
                isSubGroup = true;
                break;
            }
        }

        // Add the group if it's not member of another group
        if (! isSubGroup) {
            QTreeWidgetItem* group = new QTreeWidgetItem;
            group->setText(0, memIt1.key());
            superCategory->addChild(group);

            // Search the member list for other groups
            QMap<QString, QStringList> subGroups;
            foreach (const QString groupName, allGroups) {
                if (membersForGroup[memIt1.key()].contains(groupName)) {
                    subGroups[groupName] = membersForGroup[groupName];
                }
            }

            // If the list contains other groups, add them recursively
            if (subGroups.count() > 0) {
                addSubCategories(group, subGroups, allGroups);
            }
        }
    }
}

QString Settings::SubCategoriesPage::getCategory(QTreeWidgetItem* currentItem)
{
    while (currentItem->parent() != nullptr) {
        currentItem = currentItem->parent();
    }

    return DB::Category::unLocalizedCategoryName(currentItem->text(0));
}

void Settings::SubCategoriesPage::showTreeContextMenu(QPoint point)
{
    QTreeWidgetItem* currentItem = m_categoryTreeWidget->currentItem();
    if (currentItem == nullptr) {
        return;
    }

    m_currentSubCategory = currentItem->text(0);

    if (currentItem->parent() == nullptr) {
        // It's a top-level, "real" category
        m_currentSuperCategory = QString();
    } else {
        // It's a normal sub-category that belongs to another one
        m_currentSuperCategory = currentItem->parent()->text(0);
    }

    m_currentCategory = getCategory(currentItem);

    QMenu *menu = new QMenu;
    menu->addAction(m_newGroupAction);

    // "Real" top-level categories have to processed on the category page.
    if (m_currentSuperCategory != QString()) {
        menu->addSeparator();
        m_renameAction->setText(i18n("Rename \"%1\"", m_currentSubCategory));
        menu->addAction(m_renameAction);
        m_deleteAction->setText(i18n("Delete \"%1\"", m_currentSubCategory));
        menu->addAction(m_deleteAction);
    }

    menu->exec(m_categoryTreeWidget->mapToGlobal(point));
    delete menu;
}

void Settings::SubCategoriesPage::categoryChanged(const QString& name)
{
    if (name == QString()) {
        return;
    }

    m_membersListWidget->blockSignals(true);
    m_membersListWidget->clear();

    QStringList list = DB::ImageDB::instance()->categoryCollection()->categoryForName(name)->items();
    list += m_memberMap.groups(name);
    QStringList alreadyAdded;
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        if ((*it) == QString()) {
            // This can happen if we add group that currently has no members.
            continue;
        }

        if (! alreadyAdded.contains(*it)) {
            alreadyAdded << (*it);
            QListWidgetItem *newItem = new QListWidgetItem((*it), m_membersListWidget);
            newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
            newItem->setCheckState(Qt::Unchecked);
        }
    }

    m_currentGroup.clear();
    m_membersListWidget->clearSelection();
    m_membersListWidget->sortItems();
    m_membersListWidget->setEnabled(false);
    m_membersListWidget->blockSignals(false);
}

void Settings::SubCategoriesPage::slotGroupSelected(QTreeWidgetItem* item)
{
    // When something else than a "real" category has been selected before,
    // we have to save it's members.
    if (m_currentGroup != QString()) {
        saveOldGroup();
    }

    if (item->parent() == nullptr) {
        // A "real" category has been selected, not a group
        m_currentCategory = QString();
        m_currentGroup = QString();
        m_membersListWidget->setEnabled(false);
        categoryChanged(item->text(0));
        m_tagsInGroupLabel->setText(m_selectGroupToAddTags);
        return;
    }

    // Let's see if the category changed
    QString itemCategory = getCategory(item);
    if (m_currentCategory != itemCategory) {
        m_currentCategory = itemCategory;
        categoryChanged(m_currentCategory);
    }

    m_currentGroup = item->text(0);
    selectMembers(m_currentGroup);
    m_tagsInGroupLabel->setText(i18n("Tags in group \"%1\" of category \"%2\"",
                                     m_currentGroup,
                                     DB::Category::localizedCategoryName(m_currentCategory)));
}

void Settings::SubCategoriesPage::slotAddGroup()
{
    bool ok;
    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(m_currentCategory);
    QStringList groups = m_memberMap.groups(m_currentCategory);
    QStringList tags;
    for (QString tag : category->items())
    {
        if ( !groups.contains(tag) )
            tags << tag;
    }

    // reject existing group names:
    KStringListValidator validator(groups);
    QString newSubCategory = KInputDialog::getText(i18n("New Group"),
                                                   i18n("Group name:"),
                                                   QString() /*value*/,
                                                   &ok,
                                                   this /*parent*/,
                                                   &validator,
                                                   QString() /*mask*/,
                                                   QString() /*WhatsThis*/,
                                                   tags /*completion*/
                                                   );
    if (! ok) {
        return;
    }

    // Let's see if we already have this group
    if (groups.contains(newSubCategory)) {
        KMessageBox::sorry(this,
                           i18n("The group \"%1\" already exists.", newSubCategory),
                           i18n("Cannot add group"));
        return;
    }

    // Add the group as a new tag to the respective category
    category->addItem(newSubCategory);

    // Add the group
    m_memberMap.addGroup(m_currentCategory, newSubCategory);

    // Display the new group
    categoryChanged(m_currentCategory);

    // Display the new item
    QTreeWidgetItem* parentItem = m_categoryTreeWidget->currentItem();
    addNewSubItem(newSubCategory, parentItem);

    // Check if we also have to update some other group (in case this is not a top-level group)
    if (m_currentSuperCategory != QString()) {
        m_memberMap.addMemberToGroup(m_currentCategory, parentItem->text(0), newSubCategory);
        slotGroupSelected(parentItem);
    }
}

void Settings::SubCategoriesPage::addNewSubItem(QString& name, QTreeWidgetItem* parentItem)
{
    QTreeWidgetItem* newItem = new QTreeWidgetItem;
    newItem->setText(0, name);
    parentItem->addChild(newItem);

    if (! parentItem->isExpanded()) {
        parentItem->setExpanded(true);
    }
}

QTreeWidgetItem* Settings::SubCategoriesPage::findCategoryItem(QString category)
{
    QTreeWidgetItem* categoryItem = nullptr;
    for (int i = 0; i < m_categoryTreeWidget->topLevelItemCount(); ++i) {
        categoryItem = m_categoryTreeWidget->topLevelItem(i);
        if (DB::Category::unLocalizedCategoryName(categoryItem->text(0)) == category) {
            break;
        }
    }

    return categoryItem;
}

void Settings::SubCategoriesPage::checkItemSelection(QListWidgetItem*)
{
    saveOldGroup();
    updateCategoryTree();
}

void Settings::SubCategoriesPage::slotRenameGroup()
{
    bool ok;
    QString newSubCategoryName = KInputDialog::getText(i18n("Rename Group"),
                                                       i18n("New group name:"),
                                                       m_currentSubCategory,
                                                       &ok);

    if (! ok || m_currentSubCategory == newSubCategoryName) {
        return;
    }

    if (m_memberMap.groups(m_currentCategory).contains(newSubCategoryName)) {
        KMessageBox::sorry(this,
                            i18n("Can't rename group \"%1\" to \"%2\": this group already exists "
                                 "in category \"%2\"",
                                 m_currentSubCategory,
                                 newSubCategoryName,
                                 m_currentCategory),
                            i18n("Rename Group"));
        return;
    }

    QTreeWidgetItem *selectedGroup = m_categoryTreeWidget->currentItem();

    saveOldGroup();

    // Update the group
    m_memberMap.renameGroup(m_currentCategory, m_currentSubCategory, newSubCategoryName);

    // Update the tag in the respective category
    DB::ImageDB::instance()->categoryCollection()->categoryForName(m_currentCategory)->renameItem(m_currentSubCategory, newSubCategoryName);

    // Search for all possible sub-category items in this category that have to be renamed
    QTreeWidgetItem* categoryItem = findCategoryItem(m_currentCategory);
    for (int i = 0; i < categoryItem->childCount(); ++i) {
        renameAllSubCategories(categoryItem->child(i), m_currentSubCategory, newSubCategoryName);
    }

    // Update the displayed items
    categoryChanged(m_currentCategory);
    slotGroupSelected(selectedGroup);
}

void Settings::SubCategoriesPage::renameAllSubCategories(QTreeWidgetItem* categoryItem,
                                                         QString oldName,
                                                         QString newName)
{
    // Probably, it item itself has to be renamed
    if (categoryItem->text(0) == oldName) {
        categoryItem->setText(0, newName);
    }

    // Also check all sub-categories recursively
    for (int i = 0; i < categoryItem->childCount(); ++i) {
        renameAllSubCategories(categoryItem->child(i), oldName, newName);
    }
}

void Settings::SubCategoriesPage::slotDeleteGroup()
{
    QTreeWidgetItem* currentItem = m_categoryTreeWidget->currentItem();
    QString message;
    QString title;

    if (currentItem->childCount() > 0) {
        message = i18n("Really delete group \"%1\"? Sub-categories of this group will be "
                       "moved to the super category of \"%1\" (\"%2\"). "
                       "All other memberships of the sub-categories will stay intact.",
                       m_currentSubCategory,
                       m_currentSuperCategory);
    } else {
        message = i18n("Really delete group \"%1\"?", m_currentSubCategory);
    }

    int res = KMessageBox::warningContinueCancel(this,
                                                 message,
                                                 i18n("Delete Group"),
                                                 KGuiItem(i18n("&Delete"),
                                                 QString::fromUtf8("editdelete")));
    if (res == KMessageBox::Cancel) {
        return;
    }

    // Delete the group
    m_memberMap.deleteGroup(m_currentCategory, m_currentSubCategory);

    // Delete the tag
    DB::ImageDB::instance()->categoryCollection()->categoryForName(m_currentCategory)->removeItem(m_currentSubCategory);

    slotPageChange();
}

void Settings::SubCategoriesPage::saveOldGroup()
{
    QStringList list;
    for (int i = 0; i < m_membersListWidget->count(); ++i) {
        QListWidgetItem *item = m_membersListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            list << item->text();
        }
    }

    m_memberMap.setMembers(m_currentCategory, m_currentGroup, list);
}

void Settings::SubCategoriesPage::selectMembers(const QString& group)
{
    m_membersListWidget->blockSignals(true);
    m_membersListWidget->setEnabled(false);

    m_currentGroup = group;
    QStringList memberList = m_memberMap.members(m_currentCategory, group, false);

    for (int i = 0; i < m_membersListWidget->count(); ++i) {
        QListWidgetItem* item = m_membersListWidget->item(i);
        item->setCheckState(Qt::Unchecked);

        if (! m_memberMap.canAddMemberToGroup(m_currentCategory, group, item->text())) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        } else {
            item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            if (memberList.contains(item->text())) {
                item->setCheckState(Qt::Checked);
            }
        }
    }

    m_membersListWidget->setEnabled(true);
    m_membersListWidget->blockSignals(false);
}

void Settings::SubCategoriesPage::slotPageChange()
{
    m_tagsInGroupLabel->setText(m_selectGroupToAddTags);
    m_membersListWidget->setEnabled(false);
    m_membersListWidget->clear();
    m_currentCategory = QString();
    updateCategoryTree();
}

void Settings::SubCategoriesPage::saveSettings()
{
    saveOldGroup();
    slotPageChange();
    DB::ImageDB::instance()->memberMap() = m_memberMap;
}

void Settings::SubCategoriesPage::loadSettings()
{
    categoryChanged(m_currentCategory);
    updateCategoryTree();
}

void Settings::SubCategoriesPage::categoryRenamed(const QString& oldName, const QString& newName)
{
    if (m_currentCategory == oldName) {
        m_currentCategory = newName;
    }
}

DB::MemberMap* Settings::SubCategoriesPage::memberMap()
{
    return &m_memberMap;
}

void Settings::SubCategoriesPage::processDrop(QTreeWidgetItem* draggedItem,
                                              QTreeWidgetItem* targetItem)
{
    if (targetItem->parent() != nullptr) {
        // Dropped on a group

        // Select the group
        m_categoryTreeWidget->setCurrentItem(targetItem);
        slotGroupSelected(targetItem);

        // Check the dragged group on the member side to make it a sub-group of the target group
        m_membersListWidget->findItems(draggedItem->text(0), Qt::MatchExactly)[0]->setCheckState(Qt::Checked);
    } else {
        // Dropped on a top-level category

        // Check if it's already a direct child of the category.
        // If so, we don't need to do anything.
        QTreeWidgetItem* parent = draggedItem->parent();
        if (parent->parent() == nullptr) {
            return;
        }

        // Select the former super group
        m_categoryTreeWidget->setCurrentItem(parent);
        slotGroupSelected(parent);

        // Deselect the dragged group (this will bring it to the top level)
        m_membersListWidget->findItems(draggedItem->text(0), Qt::MatchExactly)[0]->setCheckState(Qt::Unchecked);
    }
}

void Settings::SubCategoriesPage::showMembersContextMenu(QPoint point)
{
    if (m_membersListWidget->currentItem() == nullptr) {
        return;
    }

    QMenu* menu = new QMenu;

    m_renameMemberAction->setText(i18n("Rename \"%1\"", m_membersListWidget->currentItem()->text()));
    menu->addAction(m_renameMemberAction);
    m_deleteMemberAction->setText(i18n("Delete \"%1\"", m_membersListWidget->currentItem()->text()));
    menu->addAction(m_deleteMemberAction);

    menu->exec(m_membersListWidget->mapToGlobal(point));
    delete menu;
}

void Settings::SubCategoriesPage::slotRenameMember()
{
    bool ok;
    QString newTagName = KInputDialog::getText(i18n("New Tag Name"),
                                               i18n("Tag name:"),
                                               m_membersListWidget->currentItem()->text(),
                                               &ok);
    if (! ok || newTagName == m_membersListWidget->currentItem()->text()) {
        return;
    }

    // Update the tag name in the database
    DB::ImageDB::instance()->categoryCollection()->categoryForName(m_currentCategory)->renameItem(
        m_membersListWidget->currentItem()->text(), newTagName
    );

    // Update the displayed tag name
    m_membersListWidget->currentItem()->setText(newTagName);

    // Re-order the tags, as their alphabetial order may have changed
    m_membersListWidget->sortItems();
}

void Settings::SubCategoriesPage::slotDeleteMember()
{
    QString memberToDelete = m_membersListWidget->currentItem()->text();

    // Let's see if it's a group or a normal tag
    if (m_memberMap.groups(m_currentCategory).contains(memberToDelete)) {
        // Find the tag in the tree view and select it ...
        QTreeWidgetItemIterator it(m_categoryTreeWidget);
        while (*it) {
            if ((*it)->text(0) == memberToDelete && getCategory((*it)) == m_currentCategory) {
                m_categoryTreeWidget->setCurrentItem((*it));
                m_currentSubCategory = (*it)->text(0);
                m_currentSuperCategory = (*it)->parent()->text(0);
                break;
            }
            ++it;
        }
        // ... then delete it like it had been requested by the TreeWidget's context menu
        slotDeleteGroup();
    } else {
        int res = KMessageBox::warningContinueCancel(this,
            i18n("<p>Do you really want to delete \"%1\"?<br/>"
                 "Deleting the item will remove any information "
                 "about it from any image containing the item.</p>",
                 memberToDelete),
            i18n("Really Delete %1?", memberToDelete),
            KGuiItem(i18n("&Delete"), QString::fromUtf8("editdelete"))
        );
        if (res == KMessageBox::Continue) {
            QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
            for(QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it) {
                if ((*it)->text() == m_currentCategory) {
                    (*it)->removeItem(memberToDelete);
                    break;
                }
            }
            slotPageChange();
        }
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
