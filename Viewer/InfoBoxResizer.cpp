/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
