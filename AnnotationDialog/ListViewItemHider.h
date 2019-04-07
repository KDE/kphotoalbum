/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef LISTVIEWITEMHIDER_H
#define LISTVIEWITEMHIDER_H
#include "enums.h"
class QTreeWidget;
class QTreeWidgetItem;
#include <QString>

namespace AnnotationDialog {

class ListViewItemHider
{
protected:
    ListViewItemHider() {}
    virtual ~ListViewItemHider(){}

    bool setItemsVisible( QTreeWidgetItem* parentItem );
    virtual bool shouldItemBeShown( QTreeWidgetItem* ) = 0;
};


class ListViewTextMatchHider :public ListViewItemHider
{
public:
    ListViewTextMatchHider( const QString& text, const MatchType mt, QTreeWidget* listView );

protected:
    bool shouldItemBeShown( QTreeWidgetItem* ) override;

private:
    QString m_text;
    const MatchType m_matchType;
};

class ListViewCheckedHider :public ListViewItemHider
{
public:
    explicit ListViewCheckedHider( QTreeWidget* );

protected:
    bool shouldItemBeShown( QTreeWidgetItem* ) override;
};

}

#endif /* LISTVIEWITEMHIDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
