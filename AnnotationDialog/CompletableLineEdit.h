// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H
#define ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H

#include <kpabase/enums.h>

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
    QTreeWidget *m_listView = nullptr;
    UsageMode m_mode = UsageMode::InputSingleImageConfigMode;
    ListSelect *m_listSelect = nullptr;
};
}

#endif /* ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
