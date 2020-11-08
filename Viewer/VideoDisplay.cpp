/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoDisplay.h"

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <MainWindow/FeatureDialog.h>

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
#include <phonon/seekslider.h>
#include <phonon/videowidget.h>
#include <qglobal.h>
#include <qlayout.h>
#include <qtimer.h>

Viewer::VideoDisplay::VideoDisplay(QWidget *parent)
    : Viewer::AbstractDisplay(parent)
    , m_zoomType(FullZoom)
    , m_zoomFactor(1)
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);

    m_mediaObject = nullptr;
}

void Viewer::VideoDisplay::setup()
{
    m_mediaObject = new Phonon::MediaObject(this);
    Phonon::AudioOutput *audioDevice = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    Phonon::createPath(m_mediaObject, audioDevice);

    m_videoWidget = new Phonon::VideoWidget(this);
    Phonon::createPath(m_mediaObject, m_videoWidget);

    m_slider = new Phonon::SeekSlider(this);
    m_slider->setMediaObject(m_mediaObject);
    m_slider->show();
    m_mediaObject->setTickInterval(100);

    m_videoWidget->setFocus();
    m_videoWidget->resize(1024, 768);
    m_videoWidget->move(0, 0);
    m_videoWidget->show();

    connect(m_mediaObject, &Phonon::MediaObject::finished, this, &VideoDisplay::stopped);
    connect(m_mediaObject, &Phonon::MediaObject::stateChanged, this, &VideoDisplay::phononStateChanged);
}

bool Viewer::VideoDisplay::setImage(DB::ImageInfoPtr info, bool /*forward*/)
{
    if (!m_mediaObject)
        setup();

    m_info = info;
    m_mediaObject->setCurrentSource(QUrl::fromLocalFile(info->fileName().absolute()));
    m_mediaObject->play();

    return true;
}

void Viewer::VideoDisplay::zoomIn()
{
    resize(1.25);
}

void Viewer::VideoDisplay::zoomOut()
{
    resize(0.8);
}

void Viewer::VideoDisplay::zoomFull()
{
    m_zoomType = FullZoom;
    setVideoWidgetSize();
}

void Viewer::VideoDisplay::zoomPixelForPixel()
{
    m_zoomType = PixelForPixelZoom;
    m_zoomFactor = 1;
    setVideoWidgetSize();
}

void Viewer::VideoDisplay::resize(double factor)
{
    m_zoomType = FixedZoom;
    m_zoomFactor *= factor;
    setVideoWidgetSize();
}

void Viewer::VideoDisplay::resizeEvent(QResizeEvent *event)
{
    AbstractDisplay::resizeEvent(event);
    setVideoWidgetSize();
}

Viewer::VideoDisplay::~VideoDisplay()
{
    if (m_mediaObject)
        m_mediaObject->stop();
}

void Viewer::VideoDisplay::stop()
{
    if (m_mediaObject)
        m_mediaObject->stop();
}

void Viewer::VideoDisplay::playPause()
{
    if (!m_mediaObject)
        return;

    if (m_mediaObject->state() != Phonon::PlayingState)
        m_mediaObject->play();
    else
        m_mediaObject->pause();
}

QImage Viewer::VideoDisplay::screenShoot()
{
    return QGuiApplication::primaryScreen()->grabWindow(m_videoWidget->winId()).toImage();
}

void Viewer::VideoDisplay::restart()
{
    if (!m_mediaObject)
        return;

    m_mediaObject->seek(0);
    m_mediaObject->play();
}

void Viewer::VideoDisplay::seek()
{
    if (!m_mediaObject)
        return;

    QAction *action = static_cast<QAction *>(sender());
    int value = action->data().value<int>();
    m_mediaObject->seek(m_mediaObject->currentTime() + value);
}

bool Viewer::VideoDisplay::isPaused() const
{
    if (!m_mediaObject)
        return false;

    return m_mediaObject->state() == Phonon::PausedState;
}

bool Viewer::VideoDisplay::isPlaying() const
{
    if (!m_mediaObject)
        return false;

    return m_mediaObject->state() == Phonon::PlayingState;
}

void Viewer::VideoDisplay::phononStateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    setVideoWidgetSize();
    if (newState == Phonon::ErrorState) {
        KMessageBox::error(nullptr, m_mediaObject->errorString(), i18n("Error playing media"));
    }
}

void Viewer::VideoDisplay::setVideoWidgetSize()
{
    if (!m_mediaObject)
        return;

    QSize videoSize;
    if (m_zoomType == FullZoom) {
        videoSize = QSize(size().width(), size().height() - m_slider->height());
        if (m_videoWidget->sizeHint().width() > 0) {
            m_zoomFactor = videoSize.width() / m_videoWidget->sizeHint().width();
        }
    } else {
        videoSize = m_videoWidget->sizeHint();
        if (m_zoomType == FixedZoom)
            videoSize *= m_zoomFactor;
    }

    m_videoWidget->resize(videoSize);

    QPoint pos = QPoint(width() / 2, (height() - m_slider->sizeHint().height()) / 2) - QPoint(videoSize.width() / 2, videoSize.height() / 2);
    m_videoWidget->move(pos);

    m_slider->move(0, height() - m_slider->sizeHint().height());
    m_slider->resize(width(), m_slider->sizeHint().height());
}

// vi:expandtab:tabstop=4 shiftwidth=4:
