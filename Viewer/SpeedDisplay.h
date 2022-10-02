// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SPEEDDISPLAY_H
#define SPEEDDISPLAY_H

#include <QLabel>

class QTimeLine;
class QTimer;
class QLabel;
class QHBoxLayout;

namespace Viewer
{

class SpeedDisplay : public QLabel
{
    Q_OBJECT

public:
    explicit SpeedDisplay(QWidget *parent);
    void display(int);
    void start();
    void end();
    void go();

private Q_SLOTS:
    void setAlphaChannel(int alpha);
    void setAlphaChannel(int background, int label);

private:
    QTimer *m_timer;
    QTimeLine *m_timeLine;
};
}

#endif /* SPEEDDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
