// SPDX-FileCopyrightText: 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later
#include "QtMultimediaDisplay.h"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QVideoWidget>

Viewer::QtMultimediaDisplay::QtMultimediaDisplay(QWidget *parent)
    : Viewer::VideoDisplay(parent)
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_videoWidget(new QVideoWidget(this))
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
    setup();
}

void Viewer::QtMultimediaDisplay::setup()
{
    m_mediaPlayer->setAudioOutput(new QAudioOutput(m_mediaPlayer));
    m_mediaPlayer->setVideoOutput(m_videoWidget);

    // m_videoToolBar = new VideoToolBar(this);
    // m_videoToolBar->show();
    // m_mediaObject->setTickInterval(100);

    // m_videoWidget->setFocus();
    // m_videoWidget->resize(1024, 768);
    // m_videoWidget->move(0, 0);
    // m_videoWidget->show();

    // connect(m_mediaObject, &Phonon::MediaObject::finished, this, &PhononDisplay::stopped);
    // connect(m_mediaObject, &Phonon::MediaObject::stateChanged, this, &PhononDisplay::phononStateChanged);
    // connect(m_mediaObject, &Phonon::MediaObject::tick, m_videoToolBar, &VideoToolBar::setPosition);
    // connect(m_videoToolBar, &VideoToolBar::positionChanged, m_mediaObject, &Phonon::MediaObject::seek);
    // // use proxy slots to avoid a signal-loop between the VideoToolBar and the Phonon::AudioOutput
    // connect(m_videoToolBar, &VideoToolBar::volumeChanged, this, &PhononDisplay::changeVolume);
    // connect(m_audioDevice, &Phonon::AudioOutput::volumeChanged, this, &PhononDisplay::updateVolume);
    // connect(m_videoToolBar, &VideoToolBar::muted, this, &PhononDisplay::setMuted);
    // connect(m_audioDevice, &Phonon::AudioOutput::mutedChanged, this, &PhononDisplay::updateMuteState);
}

Viewer::QtMultimediaDisplay::~QtMultimediaDisplay()
{
}

// vi:expandtab:tabstop=4 shiftwidth=4:
