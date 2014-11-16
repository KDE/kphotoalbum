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

#ifndef CATEGORYTREE_H
#define CATEGORYTREE_H

// Qt includes
#include <QTreeWidget>
#include <QMimeData>

// Local includes
#include "SubCategoriesPage.h"

namespace Settings
{

class CategoryTree : public QTreeWidget
{
    Q_OBJECT

public:
    CategoryTree(QWidget* parent = 0);
    ~CategoryTree();

private: // Functions
    void mousePressEvent(QMouseEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

private: // Variables
    SubCategoriesPage* m_subCategoriesPage;
    QTreeWidgetItem* m_draggedItem;
    QString m_draggedItemCategory;
};

}

#endif // CATEGORYTREE_H

// vi:expandtab:tabstop=4 shiftwidth=4:
