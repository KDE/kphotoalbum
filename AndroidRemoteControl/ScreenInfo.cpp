// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ScreenInfo.h"
#include "Settings.h"
#include <QScreen>
#include <QTimer>
#include <cmath>

namespace RemoteControl
{

ScreenInfo &ScreenInfo::instance()
{
    static ScreenInfo instance;
    return instance;
}

void ScreenInfo::setScreen(QScreen *screen)
{
    m_screen = screen;
    QSize size = pixelForSizeInMM(100);
    m_dotsPerMM = (size.width() + size.height()) / 2 / 100;
}

QSize ScreenInfo::pixelForSizeInMM(int size) const
{
    const QSizeF mm = m_screen->physicalSize();
    const QSize pixels = screenSize();
    return QSize((size / mm.width()) * pixels.width(),
                 (size / mm.height()) * pixels.height());
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
    return pixelForSizeInMM(Settings::instance().overviewIconSize()).width();
}

ScreenInfo::ScreenInfo()
{
    connect(&Settings::instance(), &Settings::overviewIconSizeChanged, this, &ScreenInfo::updateLayout);
    connect(&Settings::instance(), &Settings::overviewIconSizeChanged, this, &ScreenInfo::overviewIconSizeChanged);
    connect(&Settings::instance(), &Settings::overviewIconSizeChanged, this, &ScreenInfo::overviewSpacingChanged);
    connect(this, &ScreenInfo::viewWidthChanged, this,
            [this]() { QTimer::singleShot(0, this, SLOT(updateLayout())); });
}

int ScreenInfo::possibleColumns()
{
    // We need 1/4 * iw on each side
    // Add to that n * iw for the icons themselves
    // and finally (n-1)/2 * iw for spaces between icons
    // That means solve the formula
    // viewWidth = 2*1/4*iw + n* iw + (n-1)/2 * iw

    const double iconWidthMM = Settings::instance().overviewIconSize();
    const int iconWidthInPx = pixelForSizeInMM(iconWidthMM).width();
    return floor(2.0 * m_viewWidth / iconWidthInPx) / 3.0;
}

int ScreenInfo::iconHeight()
{
    const int iconHeightMM = Settings::instance().overviewIconSize();
    const int iconHeight = pixelForSizeInMM(iconHeightMM).height();
    const int innerSpacing = 10; // Value from Icon.qml
    return iconHeight + innerSpacing + m_textHeight;
}

void ScreenInfo::updateLayout()
{
    if (m_categoryCount == 0 || m_viewWidth == 0)
        return;

    const int fixedIconCount = 3; // Home, Discover, View
    const int iconCount = m_categoryCount + fixedIconCount;
    const int preferredCols = ceil(sqrt(iconCount));

    int columns;
    for (columns = qMin(possibleColumns(), preferredCols); columns < possibleColumns(); ++columns) {
        const int rows = ceil(1.0 * iconCount / columns);
        const int height = (rows + 2 * 0.25 + (rows - 1) / 2) * iconHeight();
        if (height < m_viewHeight)
            break;
    }

    m_overviewColumnCount = columns;

    Q_EMIT overviewColumnCountChanged();
}

int ScreenInfo::overviewSpacing() const
{
    return overviewIconSize() / 2;
}

} // namespace RemoteControl

#include "moc_ScreenInfo.cpp"
