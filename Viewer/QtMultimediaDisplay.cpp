// SPDX-FileCopyrightText: 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later
#include "QtMultimediaDisplay.h"
#include "Logging.h"
#include "VideoToolBar.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QAudioOutput>
#include <QDebug>
#include <QMediaPlayer>
#include <QVBoxLayout>
#include <QVideoFrame>
#include <QVideoSink>
#include <QVideoWidget>

namespace
{
constexpr qreal LOUDNESS_TO_VOLTAGE_EXPONENT = qreal(0.67);
constexpr qreal VOLTAGE_TO_LOUDNESS_EXPONENT = qreal(1.0 / LOUDNESS_TO_VOLTAGE_EXPONENT);
}

Viewer::QtMultimediaDisplay::QtMultimediaDisplay(QWidget *parent)
    : Viewer::VideoDisplay(parent)
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_videoWidget(new QVideoWidget(this))
    , m_videoToolBar(new Viewer::VideoToolBar(this))
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_videoWidget, 1);
    layout->addWidget(m_videoToolBar);

    setup();
}

void Viewer::QtMultimediaDisplay::setup()
{
    m_mediaPlayer->setAudioOutput(new QAudioOutput(m_mediaPlayer));
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    updateVolume(m_mediaPlayer->audioOutput()->volume());

    m_videoWidget->setFocus();
    m_videoWidget->resize(1024, 768);
    // m_videoWidget->move(0, 0);

    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &QtMultimediaDisplay::errorOccurred);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &QtMultimediaDisplay::updatePlaybackState);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, m_videoToolBar, &Viewer::VideoToolBar::setPosition);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &QtMultimediaDisplay::updateDuration);
    connect(m_videoToolBar, &VideoToolBar::positionChanged, m_mediaPlayer, &QMediaPlayer::setPosition);
    // use proxy slots to avoid a signal-loop between the VideoToolBar and the QAudioOutput
    connect(m_videoToolBar, &VideoToolBar::volumeChanged, this, &QtMultimediaDisplay::changeVolume);
    connect(m_mediaPlayer->audioOutput(), &QAudioOutput::volumeChanged, this, &QtMultimediaDisplay::updateVolume);
    connect(m_videoToolBar, &VideoToolBar::muted, this, &QtMultimediaDisplay::setMuted);
    connect(m_mediaPlayer->audioOutput(), &QAudioOutput::mutedChanged, this, &QtMultimediaDisplay::updateMuteState);
}

void Viewer::QtMultimediaDisplay::updatePlaybackState(QMediaPlayer::PlaybackState newState)
{
    if (newState == QMediaPlayer::StoppedState) {
        // jump to first frame, so that play/unpause will work as expected:
        m_mediaPlayer->setPosition(0);
        m_mediaPlayer->pause();
        Q_EMIT Viewer::VideoDisplay::stopped();
    }
}

void Viewer::QtMultimediaDisplay::updateDuration(qint64 duration)
{
    m_videoToolBar->setRange(0, duration);
}

void Viewer::QtMultimediaDisplay::updateVolume(float newVolumeVolt)
{
    const QSignalBlocker blocker { m_videoToolBar };
    const auto volume = qPow(newVolumeVolt, VOLTAGE_TO_LOUDNESS_EXPONENT) * 100.0;
    m_videoToolBar->setVolume(volume);
    qCDebug(ViewerLog) << "QtMultimediaDisplay volume is now at" << volume;
}

void Viewer::QtMultimediaDisplay::updateMuteState(bool mute)
{
    const QSignalBlocker blocker { m_videoToolBar };
    m_videoToolBar->setMuted(mute);
    qCDebug(ViewerLog) << "QtMultimediaDisplay mute state is now" << mute;
}

void Viewer::QtMultimediaDisplay::errorOccurred(QMediaPlayer::Error, const QString &errorString)
{
    qCWarning(ViewerLog) << "Error playing media:" << errorString;
    KMessageBox::error(nullptr, errorString, i18n("Error playing media"));
}

Viewer::QtMultimediaDisplay::~QtMultimediaDisplay()
{
    if (m_mediaPlayer)
        m_mediaPlayer->stop();
}

bool Viewer::QtMultimediaDisplay::setImageImpl(DB::ImageInfoPtr info, bool /*forward*/)
{
    m_mediaPlayer->setSource(QUrl::fromLocalFile(info->fileName().absolute()));
    m_mediaPlayer->play();

    return true;
}

bool Viewer::QtMultimediaDisplay::isPaused() const
{
    return m_mediaPlayer->playbackState() == QMediaPlayer::PausedState;
}

bool Viewer::QtMultimediaDisplay::isPlaying() const
{
    return m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState;
}

QImage Viewer::QtMultimediaDisplay::screenShoot()
{
    return m_mediaPlayer->videoSink()->videoFrame().toImage();
}

void Viewer::QtMultimediaDisplay::relativeSeek(int msec)
{
    m_mediaPlayer->setPosition(m_mediaPlayer->position() + msec);
}

bool Viewer::QtMultimediaDisplay::canRotate()
{
    return false;
}

void Viewer::QtMultimediaDisplay::stop()
{
    m_mediaPlayer->stop();
}

void Viewer::QtMultimediaDisplay::playPause()
{
    if (m_mediaPlayer->isPlaying())
        m_mediaPlayer->pause();
    else
        m_mediaPlayer->play();
}

void Viewer::QtMultimediaDisplay::restart()
{
    m_mediaPlayer->setPosition(0);
    m_mediaPlayer->play();
}

void Viewer::QtMultimediaDisplay::rotate(const DB::ImageInfoPtr & /*info*/)
{
    // not supported
}

void Viewer::QtMultimediaDisplay::changeVolume(int newVolumePercent)
{
    auto *audioOutput = m_mediaPlayer->audioOutput();
    audioOutput->setVolume(qPow(newVolumePercent / 100.0, LOUDNESS_TO_VOLTAGE_EXPONENT));
}

void Viewer::QtMultimediaDisplay::setMuted(bool mute)
{
    auto *audioOutput = m_mediaPlayer->audioOutput();
    audioOutput->setMuted(mute);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
