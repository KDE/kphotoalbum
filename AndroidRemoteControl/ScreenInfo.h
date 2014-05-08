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

#ifndef REMOTECONTROL_SCREENINFO_H
#define REMOTECONTROL_SCREENINFO_H

#include <QObject>
#include <QSize>
class QScreen;

namespace RemoteControl {

class ScreenInfo :public QObject
{
    Q_OBJECT
    Q_PROPERTY(double dotsPerMM MEMBER m_dotsPerMM CONSTANT)
    Q_PROPERTY(int overviewIconSize READ overviewIconSize WRITE setOverviewIconSize NOTIFY overviewIconSizeChanged)
    Q_PROPERTY(int overviewColumnCount MEMBER m_overviewColumnCount NOTIFY overviewColumnCountChanged)
    Q_PROPERTY(int overviewSpacing MEMBER m_overviewSpacing NOTIFY overviewSpacingChanged)
    Q_PROPERTY(int viewWidth READ viewWidth WRITE setViewWidth NOTIFY viewWidthChanged)
    Q_PROPERTY(int viewWidth MEMBER m_viewWidth NOTIFY viewWidthChanged)
    Q_PROPERTY(int viewHeight MEMBER m_viewHeight NOTIFY viewHeightChanged)

public:
    static ScreenInfo& instance();
    void setScreen(QScreen*);
    QSize pixelForSizeInMM(int width, int height);
    void setCategoryCount(int count);
    QSize screenSize() const;
    QSize viewSize() const;

    int overviewIconSize() const;
    int viewWidth() const;

public slots:
    void setOverviewIconSize(int size);
    void setViewWidth(int width);

private:
    void updateLayout();
signals:
    void overviewIconSizeChanged();    
    void overviewColumnCountChanged();
    void overviewSpacingChanged();
    void viewWidthChanged();
    void viewHeightChanged();

private:
    ScreenInfo() = default;
    QScreen* m_screen;
    double m_dotsPerMM;
    int m_categoryCount = 0;
    int m_overviewIconSize = 0;
    int m_overviewColumnCount = 0;
    int m_overviewSpacing = 0;
    int m_viewWidth = 0;
    int m_viewHeight;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SCREENINFO_H
