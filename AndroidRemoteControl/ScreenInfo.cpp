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

int ScreenInfo::overviewIconSize() const
{
    return m_overviewIconSize;
}

int ScreenInfo::overviewScreenWidth() const
{
    return m_overviewScreenWidth;
}

void ScreenInfo::setOverviewIconSize(int size)
{
    if (m_overviewIconSize != size) {
        m_overviewIconSize = size;
        emit overviewIconSizeChanged();
    }
}

void ScreenInfo::setOverviewScreenWidth(int width)
{
    if (m_overviewScreenWidth != width) {
        m_overviewScreenWidth = width;
        updateLayout();
        emit overviewScreenWidthChanged();
    }
}

void ScreenInfo::updateLayout()
{
    if (m_categoryCount == 0 || m_overviewScreenWidth == 0)
        return;

    m_overviewIconSize = pixelForSizeInMM(20,20).width();
    m_overviewSpacing = m_overviewIconSize/2;

    const int possibleCols = floor(2.0 * m_overviewScreenWidth / m_overviewIconSize +1) / 3.0;
    qDebug("%d %d => %d", m_overviewScreenWidth, m_overviewIconSize, possibleCols);
    const int preferredCols = ceil(sqrt(m_categoryCount+2));

    m_overviewColumnCount = possibleCols;

    emit overviewIconSizeChanged();
    emit overviewSpacingChanged();
    emit overviewColumnCountChanged();

    qDebug("Columns %d", m_overviewColumnCount);
}

} // namespace RemoteControl
