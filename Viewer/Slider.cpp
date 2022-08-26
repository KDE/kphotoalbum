// SPDX-FileCopyrightText:  2006-2010 Ricardo Villalba <rvm@escomposlinux.org>
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Trolltech-FreeQtFoundation-Accepted-LGPL

#include "Slider.h"
#include <QApplication>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOption>

Slider::Slider(QWidget *parent)
    : QSlider(parent)
{
    setOrientation(Qt::Horizontal);
    setMouseTracking(true); // mouseMoveEvent without press.
}

// Function copied from qslider.cpp
inline int Slider::pick(const QPoint &pt) const
{
    return orientation() == Qt::Horizontal ? pt.x() : pt.y();
}

// Function copied from qslider.cpp and modified to make it compile
void Slider::initStyleOption_Qt430(QStyleOptionSlider *option) const
{
    if (!option)
        return;

    option->initFrom(this);
    option->subControls = QStyle::SC_None;
    option->activeSubControls = QStyle::SC_None;
    option->orientation = orientation();
    option->maximum = maximum();
    option->minimum = minimum();
    option->tickPosition = (QSlider::TickPosition)tickPosition();
    option->tickInterval = tickInterval();
    option->upsideDown = (orientation() == Qt::Horizontal) ? (invertedAppearance() != (option->direction == Qt::RightToLeft))
                                                           : (!invertedAppearance());
    option->direction = Qt::LeftToRight; // we use the upsideDown option instead
    option->sliderPosition = sliderPosition();
    option->sliderValue = value();
    option->singleStep = singleStep();
    option->pageStep = pageStep();
    if (orientation() == Qt::Horizontal)
        option->state |= QStyle::State_Horizontal;
}

// Function copied from qslider.cpp and modified to make it compile
int Slider::pixelPosToRangeValue(int pos) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    int sliderMin, sliderMax, sliderLength;
    if (orientation() == Qt::Horizontal) {
        sliderLength = sr.width();
        sliderMin = gr.x();
        sliderMax = gr.right() - sliderLength + 1;
    } else {
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;
    }
    return QStyle::sliderValueFromPosition(minimum(), maximum(), pos - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

void Slider::enterEvent(QEvent *event)
{
    emit onEnter();
    QSlider::enterEvent(event);
}

void Slider::leaveEvent(QEvent *e)
{
    emit onLeave();
    QSlider::leaveEvent(e);
}

void Slider::mouseMoveEvent(QMouseEvent *e)
{
    const int o = style()->pixelMetric(QStyle::PM_SliderLength) - 1;
    int v = QStyle::sliderValueFromPosition(minimum(), maximum(), e->pos().x() - o / 2, width() - o, false);
    emit onHover(e->globalPos(), v);
    QSlider::mouseMoveEvent(e);
}

// Based on code from qslider.cpp
void Slider::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        const QRect sliderRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        const QPoint center = sliderRect.center() - sliderRect.topLeft();
        // to take half of the slider off for the setSliderPosition call we use the center - topLeft

        if (!sliderRect.contains(e->pos())) {
            e->accept();

            int v = pixelPosToRangeValue(pick(e->pos() - center));
            setSliderPosition(v);
            triggerAction(SliderMove);
            setRepeatAction(SliderNoAction);
            emit sliderMoved(v);
            emit sliderPressed();
        } else {
            QSlider::mousePressEvent(e);
        }
    } else {
        QSlider::mousePressEvent(e);
    }
}

#include "moc_Slider.cpp"
