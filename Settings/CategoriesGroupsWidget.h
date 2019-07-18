/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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

#ifndef CATEGORIESGROUPSWIDGET_H
#define CATEGORIESGROUPSWIDGET_H

// Qt includes
#include <QTreeWidget>

namespace Settings
{

// Local classes
class TagGroupsPage;

class CategoriesGroupsWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CategoriesGroupsWidget(QWidget *parent = 0);
    ~CategoriesGroupsWidget() override;

private: // Functions
    void mousePressEvent(QMouseEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void updateHighlight(QTreeWidgetItem *target);

private: // Variables
    TagGroupsPage *m_tagGroupsPage;
    QTreeWidgetItem *m_draggedItem;
    QString m_draggedItemCategory;
    QTreeWidgetItem *m_oldTarget;
    QBrush m_backgroundNoTarget;
};

}

#endif // CATEGORIESGROUPSWIDGET_H

// vi:expandtab:tabstop=4 shiftwidth=4:
