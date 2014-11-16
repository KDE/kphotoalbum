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

// Qt includes
#include <QDropEvent>
#include <QDebug>

// Local includes
#include "CategoryTree.h"
#include "DB/Category.h"

Settings::CategoryTree::CategoryTree(QWidget* parent) : QTreeWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    m_subCategoriesPage = dynamic_cast<SubCategoriesPage*>(parentWidget());
}

Settings::CategoryTree::~CategoryTree()
{
}

void Settings::CategoryTree::mousePressEvent(QMouseEvent* event)
{
    m_draggedItem = itemAt(event->pos());

    if (m_draggedItem != nullptr) {
        if (m_draggedItem->parent() != nullptr) {
            m_draggedItemCategory = m_subCategoriesPage->getCategory(m_draggedItem);
        }
    } else {
        m_draggedItemCategory = QString();
    }

    QTreeWidget::mousePressEvent(event);
}

bool Settings::CategoryTree::checkTarget(QTreeWidgetItem* target)
{
    if (target == nullptr) {
        return false;
    }

    if (target->parent() == nullptr) {
        if (DB::Category::unLocalizedCategoryName(target->text(0)) != m_draggedItemCategory) {
            return false;
        }
    } else {
        if (m_subCategoriesPage->getCategory(m_draggedItem) != m_draggedItemCategory) {
            return false;
        }
    }

    return true;
}

void Settings::CategoryTree::dragMoveEvent(QDragMoveEvent* event)
{
    QTreeWidgetItem* target = itemAt(event->pos());
    if (checkTarget(target)) {
        event->setDropAction(Qt::MoveAction);
        QTreeWidget::dragMoveEvent(event);
    } else {
        event->setDropAction(Qt::IgnoreAction);
    }
}

void Settings::CategoryTree::dropEvent(QDropEvent* event)
{
    QTreeWidgetItem* target = itemAt(event->pos());
    if (! checkTarget(target)) {
        return;
    }

    m_subCategoriesPage->processDrop(m_draggedItem, target);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
