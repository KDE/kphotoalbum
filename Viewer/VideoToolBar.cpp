// SPDX-FileCopyrightText: 2021 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoToolBar.h"
#include "Slider.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTime>
#include <QToolButton>
#include <QToolTip>
#include <kiconloader.h>

namespace Viewer
{

VideoToolBar::VideoToolBar(QWidget *parent)
    : QWidget(parent)
{
    auto hLay = new QHBoxLayout(this);

    m_currentOffset = new QLabel;
    hLay->addWidget(m_currentOffset);

    m_offsetSlider = new Slider;
    hLay->addWidget(m_offsetSlider);

    connect(m_offsetSlider, &Slider::onHover, this, &VideoToolBar::onTimeSliderHover);
    connect(m_offsetSlider, &Slider::sliderMoved, this, &VideoToolBar::positionChanged);
    connect(m_offsetSlider, &Slider::onLeave, this, &VideoToolBar::closePreview);

    m_totalTime = new QLabel;
    hLay->addWidget(m_totalTime);

    m_muteButton = new QToolButton;
    hLay->addWidget(m_muteButton);
    connect(m_muteButton, &QToolButton::clicked, this, [this] { setMuted(!m_isMuted); });

    m_volumeSlider = new QSlider(Qt::Horizontal);
    hLay->addWidget(m_volumeSlider);
    m_volumeSlider->setMaximumWidth(200);
    m_volumeSlider->setRange(0, 100);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VideoToolBar::volumeChanged);
    setMuted(false); // force loading the icon

    m_percentageLabel = new QLabel;
    hLay->addWidget(m_percentageLabel);
    m_percentageLabel->setFixedWidth(fontMetrics().horizontalAdvance(QString::fromLatin1("100%")));
    m_percentageLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
}

int VideoToolBar::maximum() const
{
    return m_offsetSlider->maximum();
}

void VideoToolBar::setRange(int min, int max)
{
    m_offsetSlider->setRange(min, max);
    m_totalTime->setText(QTime(0, 0, 0).addMSecs(max).toString(format()));
}

void VideoToolBar::setPosition(int value)
{
    m_offsetSlider->setValue(value);
    m_currentOffset->setText(QTime(0, 0, 0).addMSecs(value).toString(format()));
}

int VideoToolBar::volume() const
{
    return m_volumeSlider->value();
}

void VideoToolBar::setVolume(int volume)
{
    m_volumeSlider->setValue(volume);
    m_percentageLabel->setText(QString::fromLatin1("%1%").arg(volume));
}

bool VideoToolBar::isMuted() const
{
    return m_isMuted;
}

void VideoToolBar::setMuted(bool b)
{
    QString icon = b ? QString::fromLatin1("audio-volume-muted") : QString::fromLatin1("audio-volume-medium");
    m_muteButton->setIcon(QIcon::fromTheme(icon)
                              .pixmap(KIconLoader::StdSizes::SizeSmall));
    m_volumeSlider->setEnabled(!b);
    if (b != m_isMuted) {
        m_isMuted = b;
        emit muted(b);
    }
}

void VideoToolBar::onTimeSliderHover(const QPoint &pos, int value)
{
    QToolTip::showText(pos, QTime(0, 0, 0).addMSecs(value).toString(format()));
}

void VideoToolBar::setSeekSliderEnabled(bool b)
{
    m_offsetSlider->setEnabled(b);
}

QString VideoToolBar::format() const
{
    if (m_offsetSlider->maximum() > 60 * 60 * 1000)
        return QString::fromLatin1("HH:mm:ss");
    else
        return QString::fromLatin1("mm:ss");
}

} // namespace Viewer

#include "moc_VideoToolBar.cpp"
