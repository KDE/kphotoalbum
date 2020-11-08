/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ShowBusyCursor.h"

#include <qapplication.h>
#include <qcursor.h>

Utilities::ShowBusyCursor::ShowBusyCursor(Qt::CursorShape shape)
{
    qApp->setOverrideCursor(QCursor(shape));
    m_active = true;
}

Utilities::ShowBusyCursor::~ShowBusyCursor()
{
    stop();
}

void Utilities::ShowBusyCursor::stop()
{
    if (m_active) {
        qApp->restoreOverrideCursor();
        m_active = false;
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
