// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SPEEDDISPLAY_H
#define SPEEDDISPLAY_H

#include <QLabel>
#include <chrono>

class QTimeLine;
class QTimer;
class QLabel;
class QHBoxLayout;

namespace Viewer
{

class TransientDisplay : public QLabel
{
    Q_OBJECT

public:
    explicit TransientDisplay(QWidget *parent);
    enum FadeAction { FadeOut,
                      NoFadeOut };
    void display(const QString &text, std::chrono::milliseconds duration = std::chrono::seconds(1), FadeAction action = FadeOut);

private:
    void go(std::chrono::milliseconds duration);
    void setAlphaChannel(int alpha);
    void setAlphaChannel(int background, int label);

    QTimer *m_timer;
    QTimeLine *m_timeLine;
    FadeAction m_nextFadeAction;
};
}

#endif /* SPEEDDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
