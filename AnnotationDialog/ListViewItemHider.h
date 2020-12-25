/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef LISTVIEWITEMHIDER_H
#define LISTVIEWITEMHIDER_H
#include "enums.h"
class QTreeWidget;
class QTreeWidgetItem;
#include <QString>

namespace AnnotationDialog
{

class ListViewItemHider
{
protected:
    ListViewItemHider() { }
    virtual ~ListViewItemHider() { }

    bool setItemsVisible(QTreeWidgetItem *parentItem);
    virtual bool shouldItemBeShown(QTreeWidgetItem *) = 0;
};

class ListViewTextMatchHider : public ListViewItemHider
{
public:
    ListViewTextMatchHider(const QString &text, const MatchType mt, QTreeWidget *listView);

protected:
    bool shouldItemBeShown(QTreeWidgetItem *) override;

private:
    QString m_text;
    const MatchType m_matchType;
};

class ListViewCheckedHider : public ListViewItemHider
{
public:
    explicit ListViewCheckedHider(QTreeWidget *);

protected:
    bool shouldItemBeShown(QTreeWidgetItem *) override;
};

}

#endif /* LISTVIEWITEMHIDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
