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

#include "TagGroupsPage.h"

// Qt includes
#include <QListWidget>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QFont>
#include <QAction>
#include <QMenu>
#include <QGridLayout>
#include <QLocale>
#include <QInputDialog>

// KDE includes
#include <KLocalizedString>
#include <KMessageBox>

// Local includes
#include "MainWindow/DirtyIndicator.h"
#include "DB/CategoryCollection.h"
#include "CategoriesGroupsWidget.h"
#include "Settings/SettingsData.h"

Settings::TagGroupsPage::TagGroupsPage(QWidget* parent) : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    // The category and group tree
    layout->addWidget(new QLabel(i18nc("@label","Categories and groups:")), 0, 0);
    m_categoryTreeWidget = new CategoriesGroupsWidget(this);
    m_categoryTreeWidget->header()->hide();
    m_categoryTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_categoryTreeWidget, 1, 0);
    connect(m_categoryTreeWidget, &CategoriesGroupsWidget::customContextMenuRequested, this, &TagGroupsPage::showTreeContextMenu);
    connect(m_categoryTreeWidget, &CategoriesGroupsWidget::itemActivated, this, &TagGroupsPage::slotGroupSelected);

    // The member list
    m_selectGroupToAddTags = i18nc("@label/rich","<strong>Select a group on the left side to add tags to it</strong>");
    m_tagsInGroupLabel = new QLabel(m_selectGroupToAddTags);
    layout->addWidget(m_tagsInGroupLabel, 0, 1);
    m_membersListWidget = new QListWidget;
    m_membersListWidget->setEnabled(false);
    m_membersListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_membersListWidget, 1, 1);
    connect(m_membersListWidget, &QListWidget::itemChanged, this, &TagGroupsPage::checkItemSelection);
    connect(m_membersListWidget, &QListWidget::customContextMenuRequested, this, &TagGroupsPage::showMembersContextMenu);

    // The "pending rename actions" label
    m_pendingChangesLabel = new QLabel(i18nc("@label/rich","<strong>There are pending changes on the categories page.<nl> "
                "Please save the changes before working on tag groups.</strong>"));
    m_pendingChangesLabel->hide();
    layout->addWidget(m_pendingChangesLabel, 2, 0, 1, 2);

    QDialog *parentDialog = qobject_cast<QDialog*>(parent);
    connect( parentDialog, &QDialog::rejected, this, &TagGroupsPage::discardChanges);

    // Context menu actions
    m_newGroupAction = new QAction(i18nc("@action:inmenu","Add group ..."), this);
    connect(m_newGroupAction, &QAction::triggered, this, &TagGroupsPage::slotAddGroup);
    m_renameAction = new QAction(this);
    connect(m_renameAction, &QAction::triggered, this, &TagGroupsPage::slotRenameGroup);
    m_deleteAction = new QAction(this);
    connect(m_deleteAction, &QAction::triggered, this, &TagGroupsPage::slotDeleteGroup);
    m_deleteMemberAction = new QAction(this);
    connect(m_deleteMemberAction, &QAction::triggered, this, &TagGroupsPage::slotDeleteMember);
    m_renameMemberAction = new QAction(this);
    connect(m_renameMemberAction, &QAction::triggered, this, &TagGroupsPage::slotRenameMember);

    m_memberMap = DB::ImageDB::instance()->memberMap();

    connect(DB::ImageDB::instance()->categoryCollection(), SIGNAL(itemRemoved(DB::Category*,QString)),
            &m_memberMap, SLOT(deleteItem(DB::Category*,QString)));

    connect(DB::ImageDB::instance()->categoryCollection(), SIGNAL(itemRenamed(DB::Category*,QString,QString)),
            &m_memberMap, SLOT(renameItem(DB::Category*,QString,QString)));

    connect(DB::ImageDB::instance()->categoryCollection(), SIGNAL(categoryRemoved(QString)),
            &m_memberMap, SLOT(deleteCategory(QString)));

    m_dataChanged = false;
}

void Settings::TagGroupsPage::updateCategoryTree()
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
    Q_FOREACH(const DB::CategoryPtr category, categories ) {
        if (category->isSpecialCategory()) {
            continue;
        }

        // Add the real categories as top-level items
        QTreeWidgetItem* topLevelItem = new QTreeWidgetItem;
        topLevelItem->setText(0, category->name());
        topLevelItem->setFlags(topLevelItem->flags() & Qt::ItemIsEnabled);
        QFont font = topLevelItem->font(0);
        font.setWeight(QFont::Bold);
        topLevelItem->setFont(0, font);
        m_categoryTreeWidget->addTopLevelItem(topLevelItem);

        // Build a map with all members for each group
        QMap<QString, QStringList> membersForGroup;
        QStringList allGroups = m_memberMap.groups(category->name());
        foreach (const QString &group, allGroups) {
            // FIXME: Why does the member map return an empty category?!
            if (group.isEmpty()) {
                continue;
            }

            QStringList allMembers = m_memberMap.members(category->name(), group, false);
            foreach (const QString &member, allMembers) {
                membersForGroup[group] << member;
            }

            // We add an empty member placeholder if the group currently has no members.
            // Otherwise, it won't be added.
            if (! membersForGroup.contains(group)) {
                membersForGroup[group] = QStringList();
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

void Settings::TagGroupsPage::addSubCategories(QTreeWidgetItem* superCategory,
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
            foreach (const QString &groupName, allGroups) {
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

QString Settings::TagGroupsPage::getCategory(QTreeWidgetItem* currentItem)
{
    while (currentItem->parent() != nullptr) {
        currentItem = currentItem->parent();
    }

    return currentItem->text(0);
}

void Settings::TagGroupsPage::showTreeContextMenu(QPoint point)
{
    QTreeWidgetItem* currentItem = m_categoryTreeWidget->currentItem();
    if (currentItem == nullptr) {
        return;
    }

    m_currentSubCategory = currentItem->text(0);

    if (currentItem->parent() == nullptr) {
        // It's a top-level, "real" category
        m_currentSuperCategory.clear();
    } else {
        // It's a normal sub-category that belongs to another one
        m_currentSuperCategory = currentItem->parent()->text(0);
    }

    m_currentCategory = getCategory(currentItem);

    QMenu *menu = new QMenu;
    menu->addAction(m_newGroupAction);

    // "Real" top-level categories have to processed on the category page.
    if (!m_currentSuperCategory.isEmpty()) {
        menu->addSeparator();
        m_renameAction->setText(i18nc("@action:inmenu","Rename group \"%1\"", m_currentSubCategory));
        menu->addAction(m_renameAction);
        m_deleteAction->setText(i18nc("@action:inmenu","Delete group \"%1\"", m_currentSubCategory));
        menu->addAction(m_deleteAction);
    }

    menu->exec(m_categoryTreeWidget->mapToGlobal(point));
    delete menu;
}

void Settings::TagGroupsPage::categoryChanged(const QString& name)
{
    if (name.isEmpty()) {
        return;
    }

    m_membersListWidget->blockSignals(true);
    m_membersListWidget->clear();

    QStringList list = getCategoryObject(name)->items();
    list += m_memberMap.groups(name);
    QStringList alreadyAdded;

    Q_FOREACH( const QString &member, list ) {
        if (member.isEmpty()) {
            // This can happen if we add group that currently has no members.
            continue;
        }

        if (! alreadyAdded.contains(member)) {
            alreadyAdded << member;

            if (Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured()
                && ! Settings::SettingsData::instance()->untaggedImagesTagVisible()) {

                if (name == Settings::SettingsData::instance()->untaggedCategory()) {
                    if (member == Settings::SettingsData::instance()->untaggedTag()) {
                        continue;
                    }
                }
            }

            QListWidgetItem* newItem = new QListWidgetItem(member, m_membersListWidget);
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

void Settings::TagGroupsPage::slotGroupSelected(QTreeWidgetItem* item)
{
    // When something else than a "real" category has been selected before,
    // we have to save it's members.
    if (!m_currentGroup.isEmpty()) {
        saveOldGroup();
    }

    if (item->parent() == nullptr) {
        // A "real" category has been selected, not a group
        m_currentCategory.clear();
        m_currentGroup.clear();
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
    m_tagsInGroupLabel->setText(i18nc("@label","Tags in group \"%1\" of category \"%2\"",
                                     m_currentGroup, m_currentCategory));
}

void Settings::TagGroupsPage::slotAddGroup()
{
    bool ok;
    DB::CategoryPtr category = getCategoryObject(m_currentCategory);
    QStringList groups = m_memberMap.groups(m_currentCategory);
    QStringList tags;
    for (QString tag : category->items()) {
        if (! groups.contains(tag)) {
            tags << tag;
        }
    }

    //// reject existing group names:
    //KStringListValidator validator(groups);
    //QString newSubCategory = KInputDialog::getText(i18nc("@title:window","New Group"),
    //                                               i18nc("@label:textbox","Group name:"),
    //                                               QString() /*value*/,
    //                                               &ok,
    //                                               this /*parent*/,
    //                                               &validator,
    //                                               QString() /*mask*/,
    //                                               QString() /*WhatsThis*/,
    //                                               tags /*completion*/
    //                                               );
    // FIXME: KF5-port: QInputDialog does not accept a validator,
    // and KInputDialog was removed in KF5. -> Reimplement input validation using other stuff
    QString newSubCategory = QInputDialog::getText(this,
                                                   i18nc("@title:window","New Group"),
                                                   i18nc("@label:textbox","Group name:"),
                                                   QLineEdit::Normal,
                                                   QString(),
                                                   &ok
                                                   );
    if (groups.contains(newSubCategory))
        return; // only a workaround until GUI-support for validation is restored
    if (! ok) {
        return;
    }

    // Let's see if we already have this group
    if (groups.contains(newSubCategory)) {
        // (with the validator working correctly, we should not get to this point)
        KMessageBox::sorry(this,
                           i18nc("@info","<p>The group \"%1\" already exists.</p>", newSubCategory),
                           i18nc("@title:window","Cannot add group"));
        return;
    }

    // Add the group as a new tag to the respective category
    MainWindow::DirtyIndicator::suppressMarkDirty(true);
    category->addItem(newSubCategory);
    MainWindow::DirtyIndicator::suppressMarkDirty(false);
    QMap<CategoryEdit, QString> categoryChange;
    categoryChange[CategoryEdit::Category] = m_currentCategory;
    categoryChange[CategoryEdit::Add] = newSubCategory;
    m_categoryChanges.append(categoryChange);
    m_dataChanged = true;

    // Add the group
    m_memberMap.addGroup(m_currentCategory, newSubCategory);

    // Display the new group
    categoryChanged(m_currentCategory);

    // Display the new item
    QTreeWidgetItem* parentItem = m_categoryTreeWidget->currentItem();
    addNewSubItem(newSubCategory, parentItem);

    // Check if we also have to update some other group (in case this is not a top-level group)
    if (!m_currentSuperCategory.isEmpty()) {
        m_memberMap.addMemberToGroup(m_currentCategory, parentItem->text(0), newSubCategory);
        slotGroupSelected(parentItem);
    }

    m_dataChanged = true;
}

void Settings::TagGroupsPage::addNewSubItem(QString& name, QTreeWidgetItem* parentItem)
{
    QTreeWidgetItem* newItem = new QTreeWidgetItem;
    newItem->setText(0, name);
    parentItem->addChild(newItem);

    if (! parentItem->isExpanded()) {
        parentItem->setExpanded(true);
    }
}

QTreeWidgetItem* Settings::TagGroupsPage::findCategoryItem(QString category)
{
    QTreeWidgetItem* categoryItem = nullptr;
    for (int i = 0; i < m_categoryTreeWidget->topLevelItemCount(); ++i) {
        categoryItem = m_categoryTreeWidget->topLevelItem(i);
        if (categoryItem->text(0) == category) {
            break;
        }
    }

    return categoryItem;
}

void Settings::TagGroupsPage::checkItemSelection(QListWidgetItem*)
{
    m_dataChanged = true;
    saveOldGroup();
    updateCategoryTree();
}

void Settings::TagGroupsPage::slotRenameGroup()
{
    bool ok;
    DB::CategoryPtr category = getCategoryObject(m_currentCategory);
    QStringList groups = m_memberMap.groups(m_currentCategory);
    QStringList tags;
    for (QString tag : category->items()) {
        if (! groups.contains(tag)) {
            tags << tag;
        }
    }

    // FIXME: reject existing group names
    QString newSubCategoryName = QInputDialog::getText(this,
                                                   i18nc("@title:window","Rename Group"),
                                                   i18nc("@label:textbox","New group name:"),
                                                   QLineEdit::Normal,
                                                   m_currentSubCategory,
                                                   &ok
                                                   );
    // workaround until validation with GUI support is reimplemented:
    if (groups.contains(newSubCategoryName))
        return;

    if (! ok || m_currentSubCategory == newSubCategoryName) {
        return;
    }

    if (m_memberMap.groups(m_currentCategory).contains(newSubCategoryName)) {
        // (with the validator working correctly, we should not get to this point)
        KMessageBox::sorry(this,
                            i18nc("@info","<para>Cannot rename group \"%1\" to \"%2\": "
                                "\"%2\" already exists in category \"%3\"</para>",
                                 m_currentSubCategory,
                                 newSubCategoryName,
                                 m_currentCategory),
                            i18nc("@title:window","Rename Group"));
        return;
    }

    QTreeWidgetItem* selectedGroup = m_categoryTreeWidget->currentItem();

    saveOldGroup();

    // Update the group
    m_memberMap.renameGroup(m_currentCategory, m_currentSubCategory, newSubCategoryName);

    // Update the tag in the respective category
    MainWindow::DirtyIndicator::suppressMarkDirty(true);
    category->renameItem(m_currentSubCategory, newSubCategoryName);
    MainWindow::DirtyIndicator::suppressMarkDirty(false);
    QMap<CategoryEdit, QString> categoryChange;
    categoryChange[CategoryEdit::Category] = m_currentCategory;
    categoryChange[CategoryEdit::Rename] = m_currentSubCategory;
    categoryChange[CategoryEdit::NewName] = newSubCategoryName;
    m_categoryChanges.append(categoryChange);
    m_dataChanged = true;

    // Search for all possible sub-category items in this category that have to be renamed
    QTreeWidgetItem* categoryItem = findCategoryItem(m_currentCategory);
    for (int i = 0; i < categoryItem->childCount(); ++i) {
        renameAllSubCategories(categoryItem->child(i), m_currentSubCategory, newSubCategoryName);
    }

    // Update the displayed items
    categoryChanged(m_currentCategory);
    slotGroupSelected(selectedGroup);

    m_dataChanged = true;
}

void Settings::TagGroupsPage::renameAllSubCategories(QTreeWidgetItem* categoryItem,
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

void Settings::TagGroupsPage::slotDeleteGroup()
{
    QTreeWidgetItem* currentItem = m_categoryTreeWidget->currentItem();
    QString message;
    QString title;

    if (currentItem->childCount() > 0) {
        message = i18nc("@info","<para>Really delete group \"%1\"?</para>"
                "<para>Sub-categories of this group will be moved to the super category of \"%1\" (\"%2\").<nl/> "
                "All other memberships of the sub-categories will stay intact.</para>",
                m_currentSubCategory,
                m_currentSuperCategory);
    } else {
        message = i18nc("@info","<para>Really delete group \"%1\"?</para>", m_currentSubCategory);
    }

    int res = KMessageBox::warningContinueCancel(this,
                                                 message,
                                                 i18nc("@title:window","Delete Group"),
                                                 KGuiItem(i18n("&Delete"),
                                                 QString::fromUtf8("editdelete")));
    if (res == KMessageBox::Cancel) {
        return;
    }

    // Delete the group
    m_memberMap.deleteGroup(m_currentCategory, m_currentSubCategory);

    // Delete the tag
    MainWindow::DirtyIndicator::suppressMarkDirty(true);
    getCategoryObject(m_currentCategory)->removeItem(m_currentSubCategory);
    MainWindow::DirtyIndicator::suppressMarkDirty(false);
    QMap<CategoryEdit, QString> categoryChange;
    categoryChange[CategoryEdit::Category] = m_currentCategory;
    categoryChange[CategoryEdit::Remove] = m_currentSubCategory;
    m_categoryChanges.append(categoryChange);
    m_dataChanged = true;

    slotPageChange();

    m_dataChanged = true;
}

void Settings::TagGroupsPage::saveOldGroup()
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

void Settings::TagGroupsPage::selectMembers(const QString& group)
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

void Settings::TagGroupsPage::slotPageChange()
{
    m_tagsInGroupLabel->setText(m_selectGroupToAddTags);
    m_membersListWidget->setEnabled(false);
    m_membersListWidget->clear();
    m_currentCategory.clear();
    updateCategoryTree();
}

void Settings::TagGroupsPage::saveSettings()
{
    saveOldGroup();
    slotPageChange();
    DB::ImageDB::instance()->memberMap() = m_memberMap;
    m_categoryChanges.clear();

    if (m_dataChanged) {
        m_dataChanged = false;
        MainWindow::DirtyIndicator::markDirty();
    }

    m_categoryTreeWidget->setEnabled(true);
    m_membersListWidget->setEnabled(true);
    m_pendingChangesLabel->hide();
}

void Settings::TagGroupsPage::discardChanges()
{
    m_memberMap = DB::ImageDB::instance()->memberMap();
    slotPageChange();
    m_dataChanged = false;

    // Revert all changes to the "real" category objects
    MainWindow::DirtyIndicator::suppressMarkDirty(true);
    for (int i = m_categoryChanges.size() - 1; i >= 0; i--) {
        DB::CategoryPtr category = getCategoryObject(m_categoryChanges.at(i)[CategoryEdit::Category]);

        if (m_categoryChanges.at(i).contains(CategoryEdit::Add)) {
            // Remove added tags
            category->removeItem(m_categoryChanges.at(i)[CategoryEdit::Add]);
        } else if (m_categoryChanges.at(i).contains(CategoryEdit::Remove)) {
            // Add removed tags
            category->addItem(m_categoryChanges.at(i)[CategoryEdit::Add]);
        } else if (m_categoryChanges.at(i).contains(CategoryEdit::Rename)) {
            // Re-rename tags to their old name
            category->renameItem(m_categoryChanges.at(i)[CategoryEdit::NewName],
                                 m_categoryChanges.at(i)[Rename]);
        }
    }
    MainWindow::DirtyIndicator::suppressMarkDirty(false);

    m_categoryChanges.clear();

    m_categoryTreeWidget->setEnabled(true);
    m_membersListWidget->setEnabled(true);
    m_pendingChangesLabel->hide();
}

void Settings::TagGroupsPage::loadSettings()
{
    categoryChanged(m_currentCategory);
    updateCategoryTree();
}

void Settings::TagGroupsPage::categoryChangesPending()
{
    m_categoryTreeWidget->setEnabled(false);
    m_membersListWidget->setEnabled(false);
    m_pendingChangesLabel->show();
}

DB::MemberMap* Settings::TagGroupsPage::memberMap()
{
    return &m_memberMap;
}

void Settings::TagGroupsPage::processDrop(QTreeWidgetItem* draggedItem, QTreeWidgetItem* targetItem)
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

void Settings::TagGroupsPage::showMembersContextMenu(QPoint point)
{
    if (m_membersListWidget->currentItem() == nullptr) {
        return;
    }

    QMenu* menu = new QMenu;

    m_renameMemberAction->setText(i18nc("@action:inmenu","Rename \"%1\"", m_membersListWidget->currentItem()->text()));
    menu->addAction(m_renameMemberAction);
    m_deleteMemberAction->setText(i18nc("@action:inmenu","Delete \"%1\"", m_membersListWidget->currentItem()->text()));
    menu->addAction(m_deleteMemberAction);

    menu->exec(m_membersListWidget->mapToGlobal(point));
    delete menu;
}

void Settings::TagGroupsPage::slotRenameMember()
{
    bool ok;
    QString newTagName = QInputDialog::getText(this,
                                               i18nc("@title:window","New Tag Name"),
                                               i18nc("@label:textbox","Tag name:"),
                                               QLineEdit::Normal,
                                               m_membersListWidget->currentItem()->text(),
                                               &ok);
    if (! ok || newTagName == m_membersListWidget->currentItem()->text()) {
        return;
    }

    // Update the tag name in the database
    MainWindow::DirtyIndicator::suppressMarkDirty(true);
    getCategoryObject(m_currentCategory)->renameItem(m_membersListWidget->currentItem()->text(), newTagName);
    MainWindow::DirtyIndicator::suppressMarkDirty(false);
    QMap<CategoryEdit, QString> categoryChange;
    categoryChange[CategoryEdit::Category] = m_currentCategory;
    categoryChange[CategoryEdit::Rename] = m_membersListWidget->currentItem()->text();
    categoryChange[CategoryEdit::NewName] = newTagName;
    m_categoryChanges.append(categoryChange);

    // Update the displayed tag name
    m_membersListWidget->currentItem()->setText(newTagName);

    // Re-order the tags, as their alphabetial order may have changed
    m_membersListWidget->sortItems();
}

void Settings::TagGroupsPage::slotDeleteMember()
{
    QString memberToDelete = m_membersListWidget->currentItem()->text();

    if (m_memberMap.groups(m_currentCategory).contains(memberToDelete)) {
        // The item to delete is a group

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
        // The item to delete is a normal tag
        int res = KMessageBox::warningContinueCancel(this,
            i18nc("@info","<para>Do you really want to delete \"%1\"?</para>"
                 "<para>Deleting the item will remove any information "
                 "about it from any image containing the item.</para>",
                 memberToDelete),
            i18nc("@title:window","Really delete %1?", memberToDelete),
            KGuiItem(i18n("&Delete"), QString::fromUtf8("editdelete"))
        );
        if (res != KMessageBox::Continue) {
            return;
        }

        // Delete the tag as if it had been deleted from the annotation dialog.
        getCategoryObject(m_currentCategory)->removeItem(memberToDelete);
        slotPageChange();
    }
}

DB::CategoryPtr Settings::TagGroupsPage::getCategoryObject(QString category) const
{
    return DB::ImageDB::instance()->categoryCollection()->categoryForName(category);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
