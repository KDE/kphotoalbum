/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ViewHandler.h"

#include <QMouseEvent>
#include <QRubberBand>
#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>

/**
 * \class Viewer::ViewHandler
 * \brief Mouse handler used during zooming and panning actions
 */

Viewer::ViewHandler::ViewHandler(Viewer::ImageDisplay *display)
    : QObject(display)
    , m_scale(false)
    , m_pan(false)
    , m_rubberBand(new QRubberBand(QRubberBand::Rectangle, display))
    , m_display(display)
{
}

bool Viewer::ViewHandler::mousePressEvent(QMouseEvent *e, const QPoint &unTranslatedPos, double /*scaleFactor*/)
{
    m_pan = false;
    m_scale = false;

    if ((e->button() & Qt::LeftButton)) {
        if ((e->modifiers() & Qt::ControlModifier)) {
            m_pan = true;
        } else {
            m_scale = true;
        }
    } else if (e->button() & Qt::MidButton) {
        m_pan = true;
    }

    if (m_pan) {
        // panning
        m_last = unTranslatedPos;
        qApp->setOverrideCursor(Qt::SizeAllCursor);
        m_errorX = 0;
        m_errorY = 0;
        return true;
    } else if (m_scale) {
        // scaling
        m_start = e->pos();
        m_untranslatedStart = unTranslatedPos;
        qApp->setOverrideCursor(Qt::CrossCursor);
        return true;
    } else {
        return true;
    }
}

bool Viewer::ViewHandler::mouseMoveEvent(QMouseEvent *,
                                         const QPoint &unTranslatedPos, double scaleFactor)
{
    if (m_scale) {
        m_rubberBand->setGeometry(QRect(m_untranslatedStart, unTranslatedPos).normalized());
        m_rubberBand->show();
        return true;
    } else if (m_pan) {
        // This code need to be taking the error into account, consider this situation:
        // The user moves the mouse very slowly, only 1 pixel at a time, scale factor is 3
        // Then translated delta would be 1/3 which every time would be
        // rounded down to 0, and the panning would never move any pixels.
        double deltaX = m_errorX + (m_last.x() - unTranslatedPos.x()) / scaleFactor;
        double deltaY = m_errorY + (m_last.y() - unTranslatedPos.y()) / scaleFactor;
        QPoint deltaPoint = QPoint((int)deltaX, (int)deltaY);
        m_errorX = deltaX - ((double)((int)deltaX));
        m_errorY = deltaY - ((double)((int)deltaY));

        m_display->pan(deltaPoint);
        m_last = unTranslatedPos;
        return true;
    } else
        return false;
}

bool Viewer::ViewHandler::mouseReleaseEvent(QMouseEvent *e, const QPoint & /*unTranslatedPos*/, double /*scaleFactor*/)
{
    if (m_scale) {
        qApp->restoreOverrideCursor();
        m_rubberBand->hide();
        m_scale = false;
        if ((e->pos() - m_start).manhattanLength() > 1) {
            m_display->zoom(m_start, e->pos());
            return true;
        } else
            return false;
    } else if (m_pan) {
        qApp->restoreOverrideCursor();
        m_pan = false;
        return true;
    } else
        return false;
}

void Viewer::ViewHandler::hideEvent()
{
    // In case the escape key is pressed while viewing or scaling, then we need to restore the override cursor
    // (As in that case we will not see a key release event)
    if (m_pan || m_scale) {
        qApp->restoreOverrideCursor();
        m_pan = false;
        m_scale = false;
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ViewHandler.cpp"
