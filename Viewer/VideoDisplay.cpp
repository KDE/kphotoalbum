/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include <Phonon/VideoWidget>
#include <Phonon/AudioOutput>
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
#include <Phonon/SeekSlider>
#include <Phonon/MediaObject>

Viewer::VideoDisplay::VideoDisplay( QWidget* parent )
    :Viewer::Display( parent )
{
    _mediaObject = 0;
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
// PENDING(kdab) Review
#ifdef KDAB_TEMPORARILY_REMOVED
    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    widget->resize( size() );
    widget->move(0,0);
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Sorry, not implemented: Viewer::VideoDisplay::zoomFull");
    return ;
#endif // KDAB_TEMPORARILY_REMOVED
}

void Viewer::VideoDisplay::zoomPixelForPixel()
{
    // unfortunately we have no way to ask for the image size.
    zoomFull();
}

void Viewer::VideoDisplay::resize( const float factor )
{
    Q_UNUSED( factor );
// PENDING(kdab) Review
#ifdef KDAB_TEMPORARILY_REMOVED
    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    const QSize size( static_cast<int>( factor * widget->width() ) , static_cast<int>( factor * widget->width() ) );
    widget->resize( size );
    widget->move( (width() - size.width())/2, (height() - size.height())/2 );
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Sorry, not implemented: Viewer::VideoDisplay::resize");
    return ;
#endif // KDAB_TEMPORARILY_REMOVED
}

void Viewer::VideoDisplay::resizeEvent( QResizeEvent* event )
{
    Display::resizeEvent( event );

#ifdef KDAB_TEMPORARILY_REMOVED
    if ( !_playerPart )
        return;

    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    if ( widget->width() == event->oldSize().width() || widget->height() == event->oldSize().height() )
        widget->resize( size() );
    else
        widget->resize( qMin( widget->width(), width() ), qMin( widget->height(), height() ) );
    widget->move( (width() - widget->width())/2, (height() - widget->height())/2 );
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Code commented out in Viewer::VideoDisplay::resizeEvent");
#endif //KDAB_TEMPORARILY_REMOVED
}


void Viewer::VideoDisplay::play()
{
#ifdef KDAB_TEMPORARILY_REMOVED
    if ( KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart) )
        player->play();
    else
        invokeKaffeineAction( "player_play" );
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Code commented out in Viewer::VideoDisplay::play");
#endif //KDAB_TEMPORARILY_REMOVED
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
    if ( newState == Phonon::ErrorState ) {
        QMessageBox::critical(0, i18n("Error playing media"), _mediaObject->errorString(), QMessageBox::Close);
    }
}

void Viewer::VideoDisplay::setup()
{
    qDebug("Yo1");

    _mediaObject = new Phonon::MediaObject;
    Phonon::AudioOutput* audioDevice = new Phonon::AudioOutput( Phonon::VideoCategory );
    Phonon::createPath( _mediaObject, audioDevice );

    Phonon::VideoWidget* videoDevice = new Phonon::VideoWidget;
    Phonon::createPath( _mediaObject, videoDevice );

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget( videoDevice, 1 );

    Phonon::SeekSlider* slider = new Phonon::SeekSlider(this);
    layout->addWidget( slider );
    slider->setMediaObject( _mediaObject );
    _mediaObject->setTickInterval(100);

    videoDevice->setFocus();
    connect( _mediaObject, SIGNAL( finished() ), this, SIGNAL( stopped() ) );
    connect( _mediaObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
             this, SLOT( phononStateChanged(Phonon::State, Phonon::State) ) );

}

#include "VideoDisplay.moc"
