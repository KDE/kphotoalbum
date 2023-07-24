// SPDX-FileCopyrightText: 2023 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CursorVisiabilityHandler.h"
#include <QEvent>
#include <QTimer>
#include <chrono>

using namespace std::chrono_literals;

CursorVisiabilityHandler::CursorVisiabilityHandler(QWidget *parentWidget)
    : QObject(parentWidget)
    , m_parentWidget(parentWidget)
    , m_timer(new QTimer(this))
{
    m_cursorHidingEnabled.push(true);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &CursorVisiabilityHandler::hideCursor);
    m_parentWidget->installEventFilter(this);

    const auto children = m_parentWidget->findChildren<QWidget *>();
    for (auto child : children)
        child->installEventFilter(this);
}

bool CursorVisiabilityHandler::eventFilter(QObject *watched, QEvent *event)
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

void CursorVisiabilityHandler::showCursorTemporarily()
{
    if (!m_cursorHidingEnabled.top())
        return;

    m_parentWidget->unsetCursor();
    m_timer->start(1500ms);
}

void CursorVisiabilityHandler::disableCursorHiding()
{
    m_cursorHidingEnabled.push(false);
    m_parentWidget->unsetCursor();
}

void CursorVisiabilityHandler::enableCursorHiding()
{
    m_cursorHidingEnabled.pop();
    hideCursor();
}

void CursorVisiabilityHandler::hideCursor()
{
    if (!m_cursorHidingEnabled.top())
        return;

    m_parentWidget->setCursor(Qt::BlankCursor);
}
