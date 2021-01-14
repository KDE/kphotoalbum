/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "SelectionMaintainer.h"

#include <kpabase/FileNameList.h>

ThumbnailView::SelectionMaintainer::SelectionMaintainer(ThumbnailWidget *widget, ThumbnailModel *model)
    : m_widget(widget)
    , m_model(model)
    , m_enabled(true)
{
    m_currentItem = widget->currentItem();
    m_currentRow = widget->currentIndex().row();
    m_selectedItems = widget->selection(NoExpandCollapsedStacks);
    if (m_selectedItems.isEmpty())
        m_firstRow = -1;
    else
        m_firstRow = m_model->indexOf(m_selectedItems.at(0));
}

ThumbnailView::SelectionMaintainer::~SelectionMaintainer()
{
    if (!m_enabled)
        return;

    // We need to set the current item before we set the selection
    m_widget->setCurrentItem(m_currentItem);

    // If the previous current item was deleted, then set the last item of the selection current
    // This, however, need to be an actualt item, some of the previous selected items might have been deleted.
    if (m_widget->currentItem().isNull()) {
        for (int i = m_selectedItems.size() - 1; i >= 0; --i) {
            m_widget->setCurrentItem(m_selectedItems.at(i));
            if (!m_widget->currentItem().isNull())
                break;
        }
    }

    // Now set the selection
    m_widget->select(m_selectedItems);

    // If no item is current at this point, it means that all the items of the selection
    // had been deleted, so make the item just before the previous selection start the current.
    if (m_widget->currentItem().isNull())
        m_widget->setCurrentIndex(m_model->index(m_firstRow));
}

void ThumbnailView::SelectionMaintainer::disable()
{
    m_enabled = false;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
