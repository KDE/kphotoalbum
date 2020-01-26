/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

#ifndef TAGGROUPSPAGE_H
#define TAGGROUPSPAGE_H

// Qt includes
#include <QWidget>

// Local includes
#include <DB/ImageDB.h>
#include <DB/MemberMap.h>

// Qt classes
class QListWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QListWidgetItem;

namespace Settings
{

// Local classes
class CategoriesGroupsWidget;

enum CategoryEdit {
    Category,
    Add,
    Remove,
    Rename,
    NewName
};

class TagGroupsPage : public QWidget
{
    Q_OBJECT

public:
    explicit TagGroupsPage(QWidget *parent);
    void saveSettings();
    void loadSettings();
    DB::MemberMap *memberMap();
    QString getCategory(QTreeWidgetItem *currentItem);
    void processDrop(QTreeWidgetItem *draggedItem, QTreeWidgetItem *targetItem);

public slots:
    void categoryChangesPending();
    void slotPageChange();
    void discardChanges();

private slots:
    void slotAddGroup();
    void slotDeleteGroup();
    void slotRenameGroup();
    void showTreeContextMenu(QPoint point);
    void showMembersContextMenu(QPoint point);
    void slotGroupSelected(QTreeWidgetItem *item);
    void checkItemSelection(QListWidgetItem *);
    void slotRenameMember();
    void slotDeleteMember();

private: // Functions
    void categoryChanged(const QString &name);
    void saveOldGroup();
    void selectMembers(const QString &group);
    void renameAllSubCategories(QTreeWidgetItem *categoryItem, QString oldName, QString newName);
    void updateCategoryTree();
    void addSubCategories(QTreeWidgetItem *superCategory,
                          QMap<QString, QStringList> &membersForGroup,
                          const QStringList &allGroups);
    void addNewSubItem(QString &name, QTreeWidgetItem *parentItem);
    QTreeWidgetItem *findCategoryItem(QString category);
    DB::CategoryPtr getCategoryObject(QString category) const;

private: // Variables
    DB::MemberMap m_memberMap;
    QListWidget *m_membersListWidget;
    CategoriesGroupsWidget *m_categoryTreeWidget;
    QString m_currentCategory;
    QString m_currentGroup;
    QString m_currentSubCategory;
    QString m_currentSuperCategory;
    QString m_selectGroupToAddTags;
    QAction *m_newGroupAction;
    QAction *m_renameAction;
    QAction *m_deleteAction;
    QAction *m_deleteMemberAction;
    QAction *m_renameMemberAction;
    QLabel *m_tagsInGroupLabel;
    bool m_dataChanged;
    QList<QMap<CategoryEdit, QString>> m_categoryChanges;
    QLabel *m_pendingChangesLabel;
};

}

#endif // TAGGROUPSPAGE_H

// vi:expandtab:tabstop=4 shiftwidth=4:
