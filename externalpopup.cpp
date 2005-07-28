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

#include "externalpopup.h"
#include "imageinfo.h"
#include <ktrader.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <kservice.h>
#include <kurl.h>
#include <krun.h>
#include <klocale.h>

void ExternalPopup::populate( ImageInfoPtr current, const QStringList& imageList )
{
    _list = imageList;
    _currentInfo = current;
    clear();
    KTrader::OfferList offers = KTrader::self()->query( QString::fromLatin1("image/jpeg"), QString::fromLatin1("Type == 'Application'"));

    QStringList list = QStringList() << i18n("Current Image") << i18n("All Images in Viewer");

    bool first = true;
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QLabel* label = new QLabel( *it, this );
        label->setAlignment( Qt::AlignHCenter );
        label->setLineWidth(2);
        label->setFrameStyle( QFrame::Raised | QFrame::StyledPanel );

        bool multiple = (_list.count() > 1);
        bool enabled = (first && _currentInfo ) || (!first && multiple);

        int id = insertItem( label );
        setItemEnabled( id, enabled );
        for(KTrader::OfferList::Iterator it = offers.begin(); it != offers.end(); ++it) {
            id = insertItem( (*it)->pixmap(KIcon::Toolbar), (*it)->name() );
            setItemEnabled( id, enabled );
        }
        first = false;
    }
}

void ExternalPopup::slotExecuteService( int id )
{
    QString name = text( id );
    KTrader::OfferList offers = KTrader::self()->query( QString::fromLatin1("image/jpeg"), QString::fromLatin1("Type == 'Application' and Name == '%1'").arg(name));
    Q_ASSERT( offers.count() == 1 );
    KService::Ptr ptr = offers.first();
    KURL::List lst;
    if ( (uint) indexOf(id) > count() / 2 ) {
        for( QStringList::Iterator it = _list.begin(); it != _list.end(); ++it ) {
            lst.append( KURL::fromPathOrURL(*it) );
        }
    }
    else {
        lst.append( KURL::fromPathOrURL(_currentInfo->fileName()));
    }

    KRun::run(*ptr, lst);
}

ExternalPopup::ExternalPopup( QWidget* parent, const char* name )
    :QPopupMenu( parent, name )
{
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotExecuteService( int ) ) );
}

#include "externalpopup.moc"
