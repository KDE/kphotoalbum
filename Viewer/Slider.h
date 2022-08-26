// SPDX-FileCopyrightText:  2006-2010 Ricardo Villalba <rvm@escomposlinux.org>
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Trolltech-FreeQtFoundation-Accepted-LGPL

#pragma once

#include <QSlider>

class Slider : public QSlider
{
    Q_OBJECT
public:
    Slider(QWidget *parent = 0);

signals:
    void onEnter();
    void onLeave();
    void onHover(const QPoint &pos, int value);

private:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    int pick(const QPoint &pt) const;
    int pixelPosToRangeValue(int pos) const;
    void initStyleOption_Qt430(QStyleOptionSlider *option) const;
};
