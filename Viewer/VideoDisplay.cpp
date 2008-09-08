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

    connect( _mediaObject, SIGNAL( finished() ), this, SIGNAL( stopped() ) );
    connect( _mediaObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
             this, SLOT( phononStateChanged(Phonon::State, Phonon::State) ) );
}

bool Viewer::VideoDisplay::setImage( DB::ImageInfoPtr info, bool /*forward*/ )
{
    _info = info;
    _mediaObject->setCurrentSource( info->fileName() );
    _mediaObject->play();


    return true;
}

void Viewer::VideoDisplay::showError( const ErrorType type, const QString& fileName, const QString& mimeType )
{
    const QString failed = QString::fromLatin1( "<font color=\"red\"><b>%1</b></font>" ).arg( i18n("Failed") );
    const QString OK = QString::fromLatin1( "<font color=\"green\"><b>%1</b></font>" ).arg( i18n("OK") );
    const QString untested = QString::fromLatin1( "<b>%1</b>" ).arg( i18n("Untested") );

    QString msg = i18n("<h1><b>Error Loading Video</font></b></h1>");
    msg += QString::fromLatin1( "<table cols=\"2\">" );
    msg += i18n("<tr><td>Finding mime type for file</td><td>%1</td><tr>").arg( type == NoMimeType ? failed :
                                                                                QString::fromLatin1("<font color=\"green\"><b>%1</b></font>")
                                                                                .arg(mimeType ) );

    msg += i18n("<tr><td>Getting a KPart for the mime type</td><td>%1</td></tr>", type < NoKPart ? untested : ( type == NoKPart ? failed : OK ) );
    msg += i18n("<tr><td>Getting a library for the part</tr><td>%1</td></tr>"
           , type < NoLibrary ? untested : ( type == NoLibrary ? failed : OK ) );
    msg += i18n("<tr><td>Instantiating Part</td><td>%1</td></tr>", type < NoPartInstance ? untested : (type == NoPartInstance ? failed : OK ) );
    msg += i18n("<tr><td>Fetching Widget from part</td><td>%1</td></tr>"
           , type < NoWidget ? untested : (type == NoWidget ? failed : OK ) );
    msg += QString::fromLatin1( "</table>" );

    int ret = KMessageBox::questionYesNo( this, msg, i18n( "Unable to show video %1" ,fileName ), KGuiItem(i18n("Show More Help")),
                                          KGuiItem(i18n("Close") ) );
    if ( ret == KMessageBox::Yes )
        KToolInvocation::invokeBrowser( QString::fromLatin1("http://wiki.kde.org/tiki-index.php?page=KPhotoAlbum+Video+Support"));
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
    _mediaObject->stop();
}

void Viewer::VideoDisplay::stop()
{
    _mediaObject->stop();
}

void Viewer::VideoDisplay::playPause()
{
    if ( _mediaObject->state() != Phonon::PlayingState )
        _mediaObject->play();
    else
        _mediaObject->pause();
}

void Viewer::VideoDisplay::restart()
{
    _mediaObject->seek(0);
    _mediaObject->play();
}

void Viewer::VideoDisplay::seek()
{
    QAction* action = static_cast<QAction*>(sender());
    int value = action->data().value<int>();
    _mediaObject->seek( value );
}

bool Viewer::VideoDisplay::isPaused() const
{
    return _mediaObject->state() == Phonon::PausedState;
}

bool Viewer::VideoDisplay::isPlaying() const
{
    return _mediaObject->state() == Phonon::PlayingState;
}

void Viewer::VideoDisplay::phononStateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    if ( newState == Phonon::ErrorState ) {
        QMessageBox::critical(0, "Error playing media", _mediaObject->errorString(), QMessageBox::Close);
    }
}

#include "VideoDisplay.moc"
