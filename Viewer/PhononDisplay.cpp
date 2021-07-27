// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PhononDisplay.h"

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <MainWindow/FeatureDialog.h>

#include "VideoToolBar.h"
#include <KLocalizedString>
#include <QAction>
#include <QGuiApplication>
#include <QResizeEvent>
#include <QScreen>
#include <kmessagebox.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>
#include <phonon/videowidget.h>
#include <qglobal.h>
#include <qlayout.h>

Viewer::PhononDisplay::PhononDisplay(QWidget *parent)
    : Viewer::VideoDisplay(parent)
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
}

void Viewer::PhononDisplay::setup()
{
    m_mediaObject = new Phonon::MediaObject(this);
    m_audioDevice = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    Phonon::createPath(m_mediaObject, m_audioDevice);

    m_videoWidget = new Phonon::VideoWidget(this);
    Phonon::createPath(m_mediaObject, m_videoWidget);

    m_videoToolBar = new VideoToolBar(this);
    m_videoToolBar->show();
    m_mediaObject->setTickInterval(100);

    m_videoWidget->setFocus();
    m_videoWidget->resize(1024, 768);
    m_videoWidget->move(0, 0);
    m_videoWidget->show();

    connect(m_mediaObject, &Phonon::MediaObject::finished, this, &PhononDisplay::stopped);
    connect(m_mediaObject, &Phonon::MediaObject::stateChanged, this, &PhononDisplay::phononStateChanged);
    connect(m_mediaObject, &Phonon::MediaObject::tick, m_videoToolBar, &VideoToolBar::setPosition);
    connect(m_videoToolBar, &VideoToolBar::positionChanged, m_mediaObject, &Phonon::MediaObject::seek);
    connect(m_videoToolBar, &VideoToolBar::muted, m_audioDevice, &Phonon::AudioOutput::setMuted);
    connect(m_videoToolBar, &VideoToolBar::volumeChanged, [this](int volume) { m_audioDevice->setVolume(volume / 100.0); });
}

bool Viewer::PhononDisplay::setImageImpl(DB::ImageInfoPtr info, bool /*forward*/)
{
    if (!m_mediaObject)
        setup();

    m_mediaObject->setCurrentSource(QUrl::fromLocalFile(info->fileName().absolute()));
    m_mediaObject->play();

    return true;
}

void Viewer::PhononDisplay::zoomIn()
{
    resize(1.25);
}

void Viewer::PhononDisplay::zoomOut()
{
    resize(0.8);
}

void Viewer::PhononDisplay::zoomFull()
{
    m_zoomType = FullZoom;
    setVideoWidgetSize();
}

void Viewer::PhononDisplay::zoomPixelForPixel()
{
    m_zoomType = PixelForPixelZoom;
    m_zoomFactor = 1;
    setVideoWidgetSize();
}

void Viewer::PhononDisplay::resize(double factor)
{
    m_zoomType = FixedZoom;
    m_zoomFactor *= factor;
    setVideoWidgetSize();
}

void Viewer::PhononDisplay::resizeEvent(QResizeEvent *event)
{
    AbstractDisplay::resizeEvent(event);
    setVideoWidgetSize();
}

Viewer::PhononDisplay::~PhononDisplay()
{
    if (m_mediaObject)
        m_mediaObject->stop();
}

void Viewer::PhononDisplay::stop()
{
    if (m_mediaObject)
        m_mediaObject->stop();
}

void Viewer::PhononDisplay::playPause()
{
    if (!m_mediaObject)
        return;

    if (m_mediaObject->state() != Phonon::PlayingState)
        m_mediaObject->play();
    else
        m_mediaObject->pause();
}

QImage Viewer::PhononDisplay::screenShoot()
{
    return QGuiApplication::primaryScreen()->grabWindow(m_videoWidget->winId()).toImage();
}

void Viewer::PhononDisplay::restart()
{
    if (!m_mediaObject)
        return;

    m_mediaObject->seek(0);
    m_mediaObject->play();
}

bool Viewer::PhononDisplay::isPaused() const
{
    if (!m_mediaObject)
        return false;

    return m_mediaObject->state() == Phonon::PausedState;
}

bool Viewer::PhononDisplay::isPlaying() const
{
    if (!m_mediaObject)
        return false;

    return m_mediaObject->state() == Phonon::PlayingState;
}

void Viewer::PhononDisplay::phononStateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    setVideoWidgetSize();
    if (newState == Phonon::ErrorState) {
        KMessageBox::error(nullptr, m_mediaObject->errorString(), i18n("Error playing media"));
    }
}

void Viewer::PhononDisplay::setVideoWidgetSize()
{
    if (!m_mediaObject)
        return;

    QSize videoSize;
    if (m_zoomType == FullZoom) {
        videoSize = QSize(size().width(), size().height() - m_videoToolBar->height());
        if (m_videoWidget->sizeHint().width() > 0) {
            m_zoomFactor = videoSize.width() / m_videoWidget->sizeHint().width();
        }
    } else {
        videoSize = m_videoWidget->sizeHint();
        if (m_zoomType == FixedZoom)
            videoSize *= m_zoomFactor;
    }

    m_videoWidget->resize(videoSize);

    QPoint pos = QPoint(width() / 2, (height() - m_videoToolBar->sizeHint().height()) / 2) - QPoint(videoSize.width() / 2, videoSize.height() / 2);
    m_videoWidget->move(pos);

    m_videoToolBar->move(0, height() - m_videoToolBar->sizeHint().height());
    m_videoToolBar->resize(width(), m_videoToolBar->sizeHint().height());
    m_videoToolBar->setRange(0, m_mediaObject->totalTime());
    m_videoToolBar->setVolume(m_audioDevice->volume() * 100.0);
}

void Viewer::PhononDisplay::rotate(const DB::ImageInfoPtr & /*info*/)
{
    // Not supported.
}

void Viewer::PhononDisplay::relativeSeek(int msec)
{
    m_mediaObject->seek(m_mediaObject->currentTime() + msec);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
