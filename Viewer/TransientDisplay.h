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
    /**
     * @brief displayRating shows a 0 to 5 star rating display for the specified amount of time.
     * The rating can happen in half-steps, e.g. a rating of 9 translates to 4 and a half stars.
     * @param rating a rating between 0 and 10
     * @param duration the duration for the label to stay visible
     * @param action fade out or disappear at once
     */
    void displayRating(short rating, std::chrono::milliseconds duration = std::chrono::seconds(1), FadeAction action = FadeOut);

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
