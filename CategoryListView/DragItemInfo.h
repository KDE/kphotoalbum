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
#ifndef CATEGORYLISTVIEW_DRAGITEMINFO_H
#define CATEGORYLISTVIEW_DRAGITEMINFO_H

#include <qhash.h>
#include <qpair.h>
#include <qstring.h>

namespace CategoryListView
{

class DragItemInfo
{
public:
    DragItemInfo();
    DragItemInfo(const QString &parent, const QString &child);
    QString parent() const;
    QString child() const;
    void setParent(const QString &str);
    void setChild(const QString &str);
    bool operator<(const DragItemInfo &other) const;

private:
    QString m_parent;
    QString m_child;
};

inline bool operator==(const DragItemInfo &v1, const DragItemInfo &v2)
{
    return (v1.parent() == v2.parent()) && (v1.child() == v2.child());
}

inline uint qHash(const DragItemInfo &key)
{
    return qHash(QPair<QString, QString>(key.parent(), key.child()));
}

QDataStream &operator<<(QDataStream &stream, const DragItemInfo &);
QDataStream &operator>>(QDataStream &stream, DragItemInfo &);

typedef QSet<DragItemInfo> DragItemInfoSet;
}

#endif /* CATEGORYLISTVIEW_DRAGITEMINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
