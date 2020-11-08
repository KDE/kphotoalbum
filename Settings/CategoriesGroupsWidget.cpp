/* SPDX-FileCopyrightText: 2014-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
    } else {
        // The target has to be in the same category as the dragged group.
        QTreeWidgetItem *parent = target;
        // the category is the root item:
        while (parent->parent() != nullptr) {
            parent = parent->parent();
            if (parent == m_draggedItem) {
                event->setDropAction(Qt::IgnoreAction);
                return;
            }
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
