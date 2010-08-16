/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "VideoDisplay.h"
#include <phonon/videowidget.h>
#include <phonon/audiooutput.h>
#include <KActionCollection>
#include <qglobal.h>
#include <KServiceTypeTrader>
#include <QHBoxLayout>
#include <KMimeType>
#include <DB/ImageInfoPtr.h>
#include <DB/ImageInfo.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qtimer.h>
#include <QResizeEvent>
#include <MainWindow/FeatureDialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <ktoolbar.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kxmlguifactory.h>
#include <kmenu.h>
#include <kaction.h>
#include <ktoolinvocation.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>

Viewer::VideoDisplay::VideoDisplay( QWidget* parent )
    :Viewer::Display( parent ), _zoomType( FullZoom ), _zoomFactor(1)
{
    QPalette pal = palette();
    pal.setColor( QPalette::Window, Qt::black );
    setPalette( pal );
    setAutoFillBackground( true );

    _mediaObject = 0;
}

void Viewer::VideoDisplay::setup()
{
    _mediaObject = new Phonon::MediaObject(this);
    Phonon::AudioOutput* audioDevice =
        new Phonon::AudioOutput( Phonon::VideoCategory, this );
    Phonon::createPath( _mediaObject, audioDevice );

    _videoWidget = new Phonon::VideoWidget(this);
    Phonon::createPath( _mediaObject, _videoWidget );

    _slider = new Phonon::SeekSlider(this);
    _slider->setMediaObject( _mediaObject );
    _slider->show();
    _mediaObject->setTickInterval(100);

    _videoWidget->setFocus();
    _videoWidget->resize(1024,768 );
    _videoWidget->move(0,0);
    _videoWidget->show();


    connect( _mediaObject, SIGNAL( finished() ), this, SIGNAL( stopped() ) );
    connect( _mediaObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
             this, SLOT( phononStateChanged(Phonon::State, Phonon::State) ) );
}

bool Viewer::VideoDisplay::setImage( DB::ImageInfoPtr info, bool /*forward*/ )
{
    if ( !_mediaObject )
        setup();

    _info = info;
    _mediaObject->setCurrentSource( info->fileName(DB::AbsolutePath) );
    _mediaObject->play();

    return true;
}

void Viewer::VideoDisplay::zoomIn()
{
    resize( 1.25 );
}

void Viewer::VideoDisplay::zoomOut()
{
    resize( 0.8 );
}

void Viewer::VideoDisplay::zoomFull()
{
    _zoomType = FullZoom;
    setVideoWidgetSize();
}

void Viewer::VideoDisplay::zoomPixelForPixel()
{
    _zoomType = PixelForPixelZoom;
    _zoomFactor = 1;
    setVideoWidgetSize();
}

void Viewer::VideoDisplay::resize( double factor )
{
    _zoomType = FixedZoom;
    _zoomFactor *= factor;
    setVideoWidgetSize();
}

void Viewer::VideoDisplay::resizeEvent( QResizeEvent* event )
{
    Display::resizeEvent( event );
    setVideoWidgetSize();
}


Viewer::VideoDisplay::~VideoDisplay()
{
    if ( _mediaObject )
        _mediaObject->stop();
}

void Viewer::VideoDisplay::stop()
{
    if ( _mediaObject )
        _mediaObject->stop();
}

void Viewer::VideoDisplay::playPause()
{
    if ( !_mediaObject )
        return;

    if ( _mediaObject->state() != Phonon::PlayingState )
        _mediaObject->play();
    else
        _mediaObject->pause();
}

void Viewer::VideoDisplay::restart()
{
    if ( !_mediaObject )
        return;

    _mediaObject->seek(0);
    _mediaObject->play();
}

void Viewer::VideoDisplay::seek()
{
    if (!_mediaObject )
        return;

    QAction* action = static_cast<QAction*>(sender());
    int value = action->data().value<int>();
    _mediaObject->seek( _mediaObject->currentTime() + value );
}

bool Viewer::VideoDisplay::isPaused() const
{
    if (!_mediaObject )
        return false;

    return _mediaObject->state() == Phonon::PausedState;
}

bool Viewer::VideoDisplay::isPlaying() const
{
    if (!_mediaObject )
        return false;

    return _mediaObject->state() == Phonon::PlayingState;
}

void Viewer::VideoDisplay::phononStateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    setVideoWidgetSize();
    if ( newState == Phonon::ErrorState ) {
        QMessageBox::critical(0, i18n("Error playing media"), _mediaObject->errorString(), QMessageBox::Close);
    }
}

void Viewer::VideoDisplay::setVideoWidgetSize()
{
    if ( !_mediaObject )
        return;

    QSize videoSize;
    if ( _zoomType == FullZoom ) {
        videoSize = QSize( size().width(), size().height() - _slider->height() );
        _zoomFactor = videoSize.width() / _videoWidget->sizeHint().width();
    }
    else {
        videoSize = _videoWidget->sizeHint();
        if ( _zoomType == FixedZoom )
            videoSize *= _zoomFactor;
    }

    _videoWidget->resize( videoSize );

    QPoint pos = QPoint( width()/2, (height()-_slider->sizeHint().height())/2 )-QPoint(videoSize.width()/2, videoSize.height()/2);
    _videoWidget->move(pos);

    _slider->move( 0, height() - _slider->sizeHint().height() );
    _slider->resize( width(), _slider->sizeHint().height() );
}

#include "VideoDisplay.moc"
