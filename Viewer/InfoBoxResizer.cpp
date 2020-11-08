/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "InfoBoxResizer.h"

#include "InfoBox.h"

Viewer::InfoBoxResizer::InfoBoxResizer(Viewer::InfoBox *infoBox)
    : m_infoBox(infoBox)
{
}

void Viewer::InfoBoxResizer::setPos(QPoint pos)
{
    QRect rect = m_infoBox->geometry();
    pos = m_infoBox->mapToParent(pos);

    if (m_left)
        rect.setLeft(pos.x());
    if (m_right)
        rect.setRight(pos.x());
    if (m_top)
        rect.setTop(pos.y());
    if (m_bottom)
        rect.setBottom(pos.y());

    if (rect.width() > 100 && rect.height() > 50)
        m_infoBox->setGeometry(rect);
}

void Viewer::InfoBoxResizer::setup(bool left, bool right, bool top, bool bottom)
{
    m_left = left;
    m_right = right;
    m_top = top;
    m_bottom = bottom;
    m_active = true;
}

void Viewer::InfoBoxResizer::deactivate()
{
    m_active = false;
}

bool Viewer::InfoBoxResizer::isActive() const
{
    return m_active;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
