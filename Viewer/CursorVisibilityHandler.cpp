// SPDX-FileCopyrightText: 2023 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CursorVisibilityHandler.h"
#include <QEvent>
#include <QTimer>
#include <chrono>

using namespace std::chrono_literals;

CursorVisibilityHandler::CursorVisibilityHandler(QWidget *parentWidget)
    : QObject(parentWidget)
    , m_parentWidget(parentWidget)
    , m_timer(new QTimer(this))
{
    m_cursorHidingEnabled.push(true);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &CursorVisibilityHandler::hideCursor);
    m_parentWidget->installEventFilter(this);

    const auto children = m_parentWidget->findChildren<QWidget *>();
    for (auto child : children)
        child->installEventFilter(this);
}

bool CursorVisibilityHandler::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        // disable cursor hiding till button release
        disableCursorHiding();
        break;

    case QEvent::MouseMove:
        // just reset the timer
        showCursorTemporarily();
        break;

    case QEvent::MouseButtonRelease:
        // enable cursor hiding and reset timer
        enableCursorHiding();
        showCursorTemporarily();
        break;

    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void CursorVisibilityHandler::showCursorTemporarily()
{
    if (!m_cursorHidingEnabled.top())
        return;

    m_parentWidget->unsetCursor();
    m_timer->start(1500ms);
}

void CursorVisibilityHandler::disableCursorHiding()
{
    m_cursorHidingEnabled.push(false);
    m_parentWidget->unsetCursor();
}

void CursorVisibilityHandler::enableCursorHiding()
{
    m_cursorHidingEnabled.pop();
    hideCursor();
}

void CursorVisibilityHandler::hideCursor()
{
    if (!m_cursorHidingEnabled.top())
        return;

    m_parentWidget->setCursor(Qt::BlankCursor);
}
