/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef SUBCATEGORIESPAGE_H
#define SUBCATEGORIESPAGE_H
#include <QWidget>
#include <DB/MemberMap.h>
class Q3ListBoxItem;
class QPushButton;
class Q3ListBox;
class QComboBox;

namespace Settings
{

class SubCategoriesPage :public QWidget
{
    Q_OBJECT

public:
    SubCategoriesPage( QWidget* parent );
    void saveSettings();
    void loadSettings();
    DB::MemberMap* memberMap();

public slots:
    void slotPageChange();
    void categoryRenamed( const QString& oldName, const QString& newName );


private slots:
    void slotCategoryChanged( const QString& );
    void slotGroupSelected( Q3ListBoxItem* );
    void slotAddGroup();
    void slotDelGroup();
    void slotRenameGroup();

private:
    void slotCategoryChanged( const QString&, bool saveGroups );
    void saveOldGroup();
    void selectMembers( const QString& );
    void setButtonStates();

private:
    QComboBox* _category;
    Q3ListBox* _groups;
    Q3ListBox* _members;
    QPushButton* _rename;
    QPushButton* _del;
    DB::MemberMap _memberMap;
    QString _currentCategory;
    QString _currentGroup;
};

}


#endif /* SUBCATEGORIESPAGE_H */

