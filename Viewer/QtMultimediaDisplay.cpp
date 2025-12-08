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

    m_videoWidget->setFocus();
    m_videoWidget->resize(1024, 768);
    // m_videoWidget->move(0, 0);

    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &QtMultimediaDisplay::errorOccurred);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &QtMultimediaDisplay::updatePlaybackState);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, m_videoToolBar, &Viewer::VideoToolBar::setPosition);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &QtMultimediaDisplay::updateDuration);
    connect(m_videoToolBar, &VideoToolBar::positionChanged, m_mediaPlayer, &QMediaPlayer::setPosition);
    // // use proxy slots to avoid a signal-loop between the VideoToolBar and the Phonon::AudioOutput
    // connect(m_videoToolBar, &VideoToolBar::volumeChanged, this, &PhononDisplay::changeVolume);
    // connect(m_audioDevice, &Phonon::AudioOutput::volumeChanged, this, &PhononDisplay::updateVolume);
    // connect(m_videoToolBar, &VideoToolBar::muted, this, &PhononDisplay::setMuted);
    // connect(m_audioDevice, &Phonon::AudioOutput::mutedChanged, this, &PhononDisplay::updateMuteState);
}

void Viewer::QtMultimediaDisplay::setVideoWidgetSize()
{
    QSize videoSize;
    videoSize = m_videoWidget->sizeHint();
    // if (m_zoomType == FullZoom) {
    //     videoSize = QSize(size().width(), size().height() - m_videoToolBar->height());
    //     if (m_videoWidget->sizeHint().width() > 0) {
    //         m_zoomFactor = videoSize.width() / m_videoWidget->sizeHint().width();
    //     }
    // } else {
    //     videoSize = m_videoWidget->sizeHint();
    //     if (m_zoomType == FixedZoom)
    //         videoSize *= m_zoomFactor;
    // }

    m_videoWidget->resize(videoSize);

    QPoint pos = QPoint(width() / 2, (height() - m_videoToolBar->sizeHint().height()) / 2) - QPoint(videoSize.width() / 2, videoSize.height() / 2);
    m_videoWidget->move(pos);

    m_videoToolBar->move(0, height() - m_videoToolBar->sizeHint().height());
    m_videoToolBar->resize(width(), m_videoToolBar->sizeHint().height());
    m_videoToolBar->setRange(0, m_mediaPlayer->duration());
}

void Viewer::QtMultimediaDisplay::resizeEvent(QResizeEvent *ev)
{
    AbstractDisplay::resizeEvent(ev);
    setVideoWidgetSize();
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
    setVideoWidgetSize();

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

void Viewer::QtMultimediaDisplay::rotate(const DB::ImageInfoPtr &info)
{
    // NOP
}

// vi:expandtab:tabstop=4 shiftwidth=4:
