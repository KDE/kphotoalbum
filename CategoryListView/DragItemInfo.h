/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
