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
#include "DragItemInfo.h"

CategoryListView::DragItemInfo::DragItemInfo(const QString &parent, const QString &child)
    : m_parent(parent)
    , m_child(child)
{
}

QString CategoryListView::DragItemInfo::parent() const
{
    return m_parent;
}

QString CategoryListView::DragItemInfo::child() const
{
    return m_child;
}

bool CategoryListView::DragItemInfo::operator<(const DragItemInfo &other) const
{
    return m_parent < other.m_parent || (m_parent == other.m_parent && m_child < other.m_child);
}

CategoryListView::DragItemInfo::DragItemInfo()
{
}

QDataStream &CategoryListView::operator<<(QDataStream &stream, const DragItemInfo &info)
{
    stream << info.parent() << info.child();
    return stream;
}

QDataStream &CategoryListView::operator>>(QDataStream &stream, DragItemInfo &info)
{
    QString str;
    stream >> str;
    info.setParent(str);
    stream >> str;
    info.setChild(str);
    return stream;
}

void CategoryListView::DragItemInfo::setParent(const QString &str)
{
    m_parent = str;
}

void CategoryListView::DragItemInfo::setChild(const QString &str)
{
    m_child = str;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
