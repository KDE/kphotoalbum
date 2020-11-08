/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SELECTIONMAINTAINER_H
#define SELECTIONMAINTAINER_H

#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

#include <DB/FileNameList.h>

namespace ThumbnailView
{

class SelectionMaintainer
{
public:
    SelectionMaintainer(ThumbnailWidget *widget, ThumbnailModel *model);
    ~SelectionMaintainer();
    void disable();

private:
    ThumbnailWidget *m_widget;
    ThumbnailModel *m_model;
    DB::FileName m_currentItem;
    int m_currentRow;
    DB::FileNameList m_selectedItems;
    int m_firstRow;
    bool m_enabled;
};

}

#endif // SELECTIONMAINTAINER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
