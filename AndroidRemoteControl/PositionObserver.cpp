#include "PositionObserver.h"
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>

namespace RemoteControl {

static QQuickView* m_view = nullptr;

QQuickItem* findItem(const QString& name)
{
    return m_view->rootObject()->findChild<QQuickItem*>(name);
}

void PositionObserver::setView(QQuickView *view)
{
    Q_ASSERT(m_view == nullptr);
    m_view = view;
}

int PositionObserver::thumbnailOffset()
{
    QVariant value;
    QMetaObject::invokeMethod(findItem("thumbnailsPage"), "getIndex", Qt::DirectConnection, Q_RETURN_ARG(QVariant, value));
    return value.value<int>();
}

void RemoteControl::PositionObserver::setThumbnailOffset(int index)
{
    findItem("thumbnailsPage")->setProperty("index", index);
}

} // namespace RemoteControl
