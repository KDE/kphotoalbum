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
#ifndef ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H
#define ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H

#include "enums.h"

#include <KLineEdit>
class QKeyEvent;
class QTreeWidget;
class QTreeWidgetItem;

namespace AnnotationDialog
{
class ListSelect;
class ResizableFrame;

class CompletableLineEdit : public KLineEdit
{

public:
    /**
     * @brief This is just a convenience constructor for the common use-case when the lineEdit is inside a ListSelect.
     * @param parent
     */
    explicit CompletableLineEdit(ListSelect *parent);
    explicit CompletableLineEdit(ListSelect *ls, QWidget *parent);
    void setListView(QTreeWidget *);
    /**
     * @brief If the mode is SearchMode, only items that exist in the ListSelect can be entered.
     * @param mode
     */
    void setMode(UsageMode mode);
    void keyPressEvent(QKeyEvent *ev) override;

protected:
    QTreeWidgetItem *findItemInListView(const QString &startWith);
    bool isSpecialKey(QKeyEvent *);
    void handleSpecialKeysInSearch(QKeyEvent *);
    bool itemMatchesText(QTreeWidgetItem *item, const QString &text);
    void selectPrevNextMatch(bool next);
    void selectItemAndUpdateLineEdit(QTreeWidgetItem *item, int itemStart, const QString &inputText);
    void mergePreviousImageSelection();

private:
    QTreeWidget *m_listView;
    UsageMode m_mode;
    ListSelect *m_listSelect;
};
}

#endif /* ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
