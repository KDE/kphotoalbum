/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
