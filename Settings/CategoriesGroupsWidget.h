// SPDX-FileCopyrightText: 2014-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
    explicit CategoriesGroupsWidget(QWidget *parent = nullptr);
    ~CategoriesGroupsWidget() override;

private: // Functions
    void mousePressEvent(QMouseEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void updateHighlight(QTreeWidgetItem *target);

private: // Variables
    TagGroupsPage *m_tagGroupsPage = nullptr;
    QTreeWidgetItem *m_draggedItem = nullptr;
    QString m_draggedItemCategory;
    QTreeWidgetItem *m_oldTarget = nullptr;
    QBrush m_backgroundNoTarget;
    const QBrush m_backgroundHiglightTarget;
};

}

#endif // CATEGORIESGROUPSWIDGET_H

// vi:expandtab:tabstop=4 shiftwidth=4:
