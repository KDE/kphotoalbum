/* Copyright (C) 2014-2020 The KPhotoAlbum Development Team

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

#include "CategoriesGroupsWidget.h"

// Qt includes
#include <QDropEvent>

// Local includes
#include "TagGroupsPage.h"

#include <DB/Category.h>

Settings::CategoriesGroupsWidget::CategoriesGroupsWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_backgroundHiglightTarget(Qt::lightGray)
{
    setDragEnabled(true);
    setAcceptDrops(true);

    m_tagGroupsPage = dynamic_cast<TagGroupsPage *>(parentWidget());
    m_oldTarget = nullptr;
}

Settings::CategoriesGroupsWidget::~CategoriesGroupsWidget()
{
}

void Settings::CategoriesGroupsWidget::mousePressEvent(QMouseEvent *event)
{
    m_draggedItem = itemAt(event->pos());

    if (m_draggedItem != nullptr) {
        if (m_draggedItem->parent() != nullptr) {
            m_draggedItemCategory = m_tagGroupsPage->getCategory(m_draggedItem);
        }
    } else {
        m_draggedItemCategory = QString();
    }

    QTreeWidget::mousePressEvent(event);
}

void Settings::CategoriesGroupsWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeWidgetItem *target = itemAt(event->pos());

    if (target == nullptr) {
        // We don't have a target, so we don't allow a drop.
        event->setDropAction(Qt::IgnoreAction);
    } else if (target->parent() == nullptr) {
        // The target is a category. It has to be the same one as dragged group's category,
        if (target->text(0) != m_draggedItemCategory) {
            event->setDropAction(Qt::IgnoreAction);
        } else {
            updateHighlight(target);
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    } else {
        // The target is another group. It has to be in the same category as the dragged group.
        QTreeWidgetItem *parent = target->parent();
        ;
        while (parent->parent() != nullptr) {
            parent = parent->parent();
        }
        if (parent->text(0) != m_draggedItemCategory) {
            event->setDropAction(Qt::IgnoreAction);
        } else {
            updateHighlight(target);
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
    }
}

void Settings::CategoriesGroupsWidget::updateHighlight(QTreeWidgetItem *target)
{
    Q_ASSERT(target != nullptr);
    if (target == m_oldTarget) {
        return;
    }

    if (m_oldTarget != nullptr) {
        m_oldTarget->setBackground(0, m_backgroundNoTarget);
    }

    m_backgroundNoTarget = target->background(0);
    target->setBackground(0, m_backgroundHiglightTarget);

    m_oldTarget = target;
}

void Settings::CategoriesGroupsWidget::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem *target = itemAt(event->pos());
    target->setBackground(0, m_backgroundNoTarget);
    m_oldTarget = nullptr;

    if (m_draggedItem != target) {
        m_tagGroupsPage->processDrop(m_draggedItem, target);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
