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

#include "ScreenInfo.h"
#include <QScreen>
#include <cmath>

namespace RemoteControl {

ScreenInfo& ScreenInfo::instance()
{
    static ScreenInfo instance;
    return instance;
}

void ScreenInfo::setScreen(QScreen* screen)
{
    m_screen = screen;
    QSize size = pixelForSizeInMM(100,100);
    m_dotsPerMM = (size.width() + size.height()) / 2 / 100;

    m_overviewIconSize = pixelForSizeInMM(20,20).width();
}

QSize ScreenInfo::pixelForSizeInMM(int width, int height)
{
    const QSizeF mm = m_screen->physicalSize();
    const QSize pixels = screenSize();
    return QSize( (width / mm.width() ) * pixels.width(),
                  (height / mm.height()) * pixels.height());
}

void ScreenInfo::setCategoryCount(int count)
{
    m_categoryCount = count;
    updateLayout();
}

QSize ScreenInfo::screenSize() const
{
    return m_screen->geometry().size();
}

QSize ScreenInfo::viewSize() const
{
    return QSize(m_viewWidth, m_viewHeight);
}

int ScreenInfo::overviewIconSize() const
{
    return m_overviewIconSize;
}

int ScreenInfo::viewWidth() const
{
    return m_viewWidth;
}

void ScreenInfo::setOverviewIconSize(int size)
{
    if (m_overviewIconSize != size) {
        m_overviewIconSize = size;
        emit overviewIconSizeChanged();
    }
}

void ScreenInfo::setViewWidth(int width)
{
    if (m_viewWidth != width) {
        m_viewWidth = width;
        updateLayout();
        emit viewWidthChanged();
    }
}

void ScreenInfo::updateLayout()
{
    if (m_categoryCount == 0 || m_viewWidth == 0)
        return;

    m_overviewSpacing = m_overviewIconSize/2;

    const int possibleCols = floor(2.0 * m_viewWidth / m_overviewIconSize +1) / 3.0;
    // qDebug("%d %d => %d", m_overviewScreenWidth, m_overviewIconSize, possibleCols);
    const int preferredCols = ceil(sqrt(m_categoryCount+2));

    m_overviewColumnCount = possibleCols;

    emit overviewIconSizeChanged();
    emit overviewSpacingChanged();
    emit overviewColumnCountChanged();

    // qDebug("Columns %d", m_overviewColumnCount);
}

} // namespace RemoteControl
