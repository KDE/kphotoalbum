/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
    // This will find the QML element PositionObserver with the given name.
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
