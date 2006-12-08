/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef LISTVIEWITEMHIDER_H
#define LISTVIEWITEMHIDER_H
#include <qlistview.h>

namespace AnnotationDialog {

class ListViewHider
{
protected:
    ListViewHider() {}
    void setItemsVisible( QListView* );
    bool setItemsVisible( QListViewItem* parentItem );
    virtual bool shouldItemBeShown( QListViewItem* ) = 0;
};


class ListViewTextMatchHider :public ListViewHider
{
public:
    ListViewTextMatchHider( const QString& text, bool anchorAtStart, QListView* listView );

protected:
    virtual bool shouldItemBeShown( QListViewItem* );

private:
    QString _text;
    bool _anchorAtStart;
};

class ListViewCheckedHider :public ListViewHider
{
public:
    ListViewCheckedHider( QListView* );

protected:
    virtual bool shouldItemBeShown( QListViewItem* );
};

}

#endif /* LISTVIEWITEMHIDER_H */

