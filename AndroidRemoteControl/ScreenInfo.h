// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTECONTROL_SCREENINFO_H
#define REMOTECONTROL_SCREENINFO_H

#include <QObject>
#include <QSize>
class QScreen;

namespace RemoteControl
{

class ScreenInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double dotsPerMM MEMBER m_dotsPerMM CONSTANT)
    Q_PROPERTY(int overviewIconSize READ overviewIconSize NOTIFY overviewIconSizeChanged)
    Q_PROPERTY(int overviewColumnCount MEMBER m_overviewColumnCount NOTIFY overviewColumnCountChanged)
    Q_PROPERTY(int overviewSpacing READ overviewSpacing NOTIFY overviewSpacingChanged)
    Q_PROPERTY(int viewWidth MEMBER m_viewWidth NOTIFY viewWidthChanged)
    Q_PROPERTY(int viewHeight MEMBER m_viewHeight NOTIFY viewHeightChanged)
    Q_PROPERTY(int textHeight MEMBER m_textHeight NOTIFY textHeightChanged)

public:
    static ScreenInfo &instance();
    void setScreen(QScreen *);
    Q_INVOKABLE QSize pixelForSizeInMM(int size) const;
    void setCategoryCount(int count);
    QSize screenSize() const;
    QSize viewSize() const;

    int overviewIconSize() const;
    int overviewSpacing() const;

Q_SIGNALS:
    void overviewIconSizeChanged();
    void overviewColumnCountChanged();
    void overviewSpacingChanged();
    void viewWidthChanged();
    void viewHeightChanged();
    void textHeightChanged();

private Q_SLOTS:
    void updateLayout();

private:
    ScreenInfo();
    int possibleColumns();
    int iconHeight();

    QScreen *m_screen = nullptr;
    double m_dotsPerMM = 0;
    int m_categoryCount = 0;
    int m_overviewColumnCount = 0;
    int m_viewWidth = 0;
    int m_viewHeight = 0;
    int m_textHeight = 0;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SCREENINFO_H
