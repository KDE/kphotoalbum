/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#include "PositionObserver.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>

namespace RemoteControl
{

static QQuickView *m_view = nullptr;

QQuickItem *findItem(const QString &name)
{
    return m_view->rootObject()->findChild<QQuickItem *>(name);
}

void PositionObserver::setView(QQuickView *view)
{
    Q_ASSERT(m_view == nullptr);
    m_view = view;
}

void setOffset(const QString &view, int index)
{
    QQuickItem *item = findItem(view);
    Q_ASSERT(item);
    item->setProperty("index", index);
}

int getOffset(const QString &view)
{
    QQuickItem *item = findItem(view);
    Q_ASSERT(item);

    QVariant value;
    QMetaObject::invokeMethod(item, "getIndex", Qt::DirectConnection, Q_RETURN_ARG(QVariant, value));

    return value.value<int>();
}

void PositionObserver::setCategoryIconViewOffset(int index)
{
    setOffset("categoryPage", index);
}

int PositionObserver::categoryIconViewOffset()
{
    return getOffset("categoryPage");
}

int PositionObserver::thumbnailOffset()
{
    return getOffset("thumbnailsPage");
}

void PositionObserver::setCategoryListViewOffset(int index)
{
    setOffset("listViewPageObserver", index);
}

int PositionObserver::categoryListViewOffset()
{
    return getOffset("listViewPageObserver");
}

void RemoteControl::PositionObserver::setThumbnailOffset(int index)
{
    setOffset("thumbnailsPage", index);
}

} // namespace RemoteControl
