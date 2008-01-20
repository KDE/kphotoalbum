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
#include <KServiceTypeTrader>
#include <QHBoxLayout>
#include <KMimeType>
#include <DB/ImageInfoPtr.h>
#include <DB/ImageInfo.h>

#include <kmediaplayer/player.h>
#ifdef TEMPORARILY_REMOVED
#include <kuserprofile.h>
#endif
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
    :Viewer::Display( parent ), _playerPart( 0 )
{
    _player = new Phonon::VideoPlayer(Phonon::VideoCategory, this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget( _player, 1 );

    Phonon::SeekSlider* slider = new Phonon::SeekSlider(this);
    layout->addWidget( slider );
    slider->setMediaObject( _player->mediaObject() );
    _player->mediaObject()->setTickInterval(100);

    connect( _player, SIGNAL( finished() ), this, SIGNAL( stopped() ) );
}

bool Viewer::VideoDisplay::setImage( DB::ImageInfoPtr info, bool /*forward*/ )
{
    _info = info;
    _player->play( info->fileName() );

#ifdef TEMPORARILY_REMOVED
    // This code is inspired by similar code in Gwenview.
    delete _playerPart;
    _playerPart = 0;

    // Figure out the mime type associated to the file file name
    QString mimeType= mimeTypeForFileName(info->fileName());
    if ( mimeType.isEmpty() ) {
        showError( NoMimeType, info->fileName(), mimeType );
        return false;
    }


//     _playerPart =  KMimeTypeTrader::createInstanceFromQuery<KParts::ReadOnlyPart>( mimeType, this, this );
    KServiceTypeProfile::OfferList services = KServiceTypeProfile::offers(mimeType, QString::fromLatin1("KParts/ReadOnlyPart"));

    ErrorType etype = NoKPart;

    for( KServiceTypeProfile::OfferList::Iterator it = services.begin(); it != services.end(); ++it ) {

       // Ask for a part for this mime type
       KService::Ptr service = (*it).service();

       if (!service.data()) {
           etype = NoKPart;
           kWarning() << "Couldn't find a KPart for " << mimeType;
           continue;
       }

       QString library=service->library();
       if ( library.isNull() ) {
           etype = NoLibrary;
           kWarning() << "The library returned from the service was null, indicating we could not display videos.";
           continue;
      }

       _playerPart = KParts::ComponentFactory::createPartInstanceFromService<KParts::ReadOnlyPart>(service, this );

       if (!_playerPart) {
           etype = NoPartInstance;
           kWarning() << "Failed to instantiate KPart from library " << library;
           continue;
       }

       QWidget* widget = _playerPart->widget();
       if ( !widget ) {
           etype = NoWidget;
           continue;
       }
       etype = NoError;
       widget->show();
       break;
    }
    if (etype != NoError) {
       showError(etype, info->fileName(), mimeType );
       return false;
    }
    if ( _playerPart ) {
        _playerPart->openUrl(info->fileName());
        _playerPart->widget()->show();
        QHBoxLayout*layout = new QHBoxLayout( this );
        layout->addWidget( _playerPart->widget() );
    }
    else
        showError( NoKPart, info->fileName(), mimeType );

    // If the part implements the KMediaPlayer::Player interface, start
    // playing (needed for Kaboodle)
    if ( KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart) )
        player->play();

    connect( _playerPart, SIGNAL( stateChanged( int ) ), this, SLOT( stateChanged( int ) ) );

    zoomFull();

#endif

    return true;
}

void Viewer::VideoDisplay::stateChanged( int state)
{
    if ( state == KMediaPlayer::Player::Stop ) {
        emit stopped();
    }
}


QString Viewer::VideoDisplay::mimeTypeForFileName( const QString& fileName ) const
{
    QString res = KMimeType::findByUrl(fileName)->name();
    if ( res == QString::fromLatin1("application/vnd.rn-realmedia") && !MainWindow::FeatureDialog::hasVideoSupport( res ) )
        res = QString::fromLatin1( "video/vnd.rn-realvideo" );

    return res;
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
    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    widget->resize( size() );
    widget->move(0,0);
}

void Viewer::VideoDisplay::zoomPixelForPixel()
{
    // unfortunately we have no way to ask for the image size.
    zoomFull();
}

void Viewer::VideoDisplay::resize( const float factor )
{
    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    const QSize size( static_cast<int>( factor * widget->width() ) , static_cast<int>( factor * widget->width() ) );
    widget->resize( size );
    widget->move( (width() - size.width())/2, (height() - size.height())/2 );
}

void Viewer::VideoDisplay::resizeEvent( QResizeEvent* event )
{
    Display::resizeEvent( event );

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
}


void Viewer::VideoDisplay::play()
{
    if ( KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart) )
        player->play();
    else
        invokeKaffeineAction( "player_play" );
}

void Viewer::VideoDisplay::stop()
{
    if ( KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart) )
        player->stop();
    else
        invokeKaffeineAction( "player_stop" );
}

void Viewer::VideoDisplay::pause()
{
    if ( KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart) )
        player->pause();
    else
        invokeKaffeineAction( "player_pause" );
}

/**
 * Kafein does not implement the KMediaPlayer::Player interface, so I have to invoke the actions by their name.
 */
void Viewer::VideoDisplay::invokeKaffeineAction( const char* actionName )
{
#ifdef TEMPORARILY_REMOVED
    KAction* action = _playerPart->action( actionName );
        if ( action )
            action->activate();

#else
        kDebug() << "TEMPORARILY REMOVED: " ;
        Q_UNUSED( actionName );
#endif
}

void Viewer::VideoDisplay::restart()
{
    if ( KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart) )
        player->seek(0);
    else
        invokeKaffeineAction( "player_play");
}

Viewer::VideoDisplay::~VideoDisplay()
{
    _player->stop();
}

#include "VideoDisplay.moc"
