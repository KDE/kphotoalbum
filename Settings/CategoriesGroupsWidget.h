/* SPDX-FileCopyrightText: 2014-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
    const QBrush m_backgroundHiglightTarget;
};

}

#endif // CATEGORIESGROUPSWIDGET_H

// vi:expandtab:tabstop=4 shiftwidth=4:
