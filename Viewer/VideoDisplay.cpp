/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "VideoDisplay.h"
#include <qimage.h>
#include <kmimetype.h>
#include <DB/ImageInfoPtr.h>
#include <DB/ImageInfo.h>

#include <kmediaplayer/player.h>
#include <kmimetype.h>
#include <kuserprofile.h>
#include <kxmlguibuilder.h>
#include <kxmlguifactory.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qtimer.h>


Viewer::VideoDisplay::VideoDisplay( QWidget* parent )
    :Viewer::Display( parent, "VideoDisplay" ), _playerPart( 0 )
{
    _layout = new QHBoxLayout( this );
#ifdef TEMPORARILY_REMOVED
    _timer = new QTimer( this );
    connect( _timer, SIGNAL( timeout() ), this, SLOT( checkState() ) );
#endif
}

void Viewer::VideoDisplay::setImage( DB::ImageInfoPtr info, bool /*forward*/ )
{
    // This code is inspired by similar code in Gwenview.
    delete _playerPart;
    _playerPart = 0;

    QString mimeType=KMimeType::findByURL(info->fileName())->name();
    KService::Ptr service = KServiceTypeProfile::preferredService(mimeType, QString::fromLatin1("KParts/ReadOnlyPart"));
    if (!service) {
        kdWarning() << "Couldn't find a KPart for " << mimeType << endl;
        return;
    }

    QString library=service->library();
    Q_ASSERT(!library.isNull());
    _playerPart = KParts::ComponentFactory::createPartInstanceFromService<KParts::ReadOnlyPart>(service, this, 0, this, 0);
    if (!_playerPart) {
        kdWarning() << "Failed to instantiate KPart from library " << library << endl;
        return;
    }
    _layout->addWidget(_playerPart->widget());
    _playerPart->openURL(info->fileName());

    // If the part implements the KMediaPlayer::Player interface, start
    // playing (needed for Kaboodle)
    KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart);
    if (player) {
        player->play();
    }

#ifdef TEMPORARILY_REMOVED
    connect( player, SIGNAL( stateChanged( int ) ), this, SLOT( stateChanged( int ) ) );
    qDebug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>OK");
    _timer->start( 500 );
#endif
}

void Viewer::VideoDisplay::stateChanged( int state)
{
    qDebug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> State changed to %d, stopped=%d", state, KMediaPlayer::Player::Stop );
    if ( state == KMediaPlayer::Player::Stop )
        emit stopped();
}

#ifdef TEMPORARILY_REMOVED
// This was an attempt at a workarround for the missing stateChanged signal, but this didn't work either.
void Viewer::VideoDisplay::checkState()
{
    qDebug(". %ld %ld", dynamic_cast<KMediaPlayer::Player *>(_playerPart)->position(), dynamic_cast<KMediaPlayer::Player *>(_playerPart)->length());
    if ( dynamic_cast<KMediaPlayer::Player *>(_playerPart)->state() == KMediaPlayer::Player::Stop ) {
        emit stopped();
        _timer->stop();
    }
}
#endif
