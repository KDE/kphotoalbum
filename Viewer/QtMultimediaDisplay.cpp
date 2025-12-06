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
#include <QVideoFrame>
#include <QVideoSink>
#include <QVideoWidget>

Viewer::QtMultimediaDisplay::QtMultimediaDisplay(QWidget *parent)
    : Viewer::VideoDisplay(parent)
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_videoWidget(new QVideoWidget(this))
    , m_videoToolBar(new Viewer::VideoToolBar(this))
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
    setup();
}

void Viewer::QtMultimediaDisplay::setup()
{
    m_mediaPlayer->setAudioOutput(new QAudioOutput(m_mediaPlayer));
    m_mediaPlayer->setVideoOutput(m_videoWidget);

    m_videoToolBar = new VideoToolBar(this);
    m_videoToolBar->show();

    m_videoWidget->setFocus();
    // m_videoWidget->resize(1024, 768);
    // m_videoWidget->move(0, 0);
    m_videoWidget->show();

    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &QtMultimediaDisplay::errorOccurred);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &QtMultimediaDisplay::updatePlaybackState);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, m_videoToolBar, &Viewer::VideoToolBar::setPosition);
    connect(m_videoToolBar, &VideoToolBar::positionChanged, m_mediaPlayer, &QMediaPlayer::setPosition);
    // // use proxy slots to avoid a signal-loop between the VideoToolBar and the Phonon::AudioOutput
    // connect(m_videoToolBar, &VideoToolBar::volumeChanged, this, &PhononDisplay::changeVolume);
    // connect(m_audioDevice, &Phonon::AudioOutput::volumeChanged, this, &PhononDisplay::updateVolume);
    // connect(m_videoToolBar, &VideoToolBar::muted, this, &PhononDisplay::setMuted);
    // connect(m_audioDevice, &Phonon::AudioOutput::mutedChanged, this, &PhononDisplay::updateMuteState);
}

void Viewer::QtMultimediaDisplay::updatePlaybackState(QMediaPlayer::PlaybackState newState)
{
    if (newState == QMediaPlayer::StoppedState)
        Q_EMIT Viewer::VideoDisplay::stopped();
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

// vi:expandtab:tabstop=4 shiftwidth=4:
