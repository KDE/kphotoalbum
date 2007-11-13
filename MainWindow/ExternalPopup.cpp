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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ExternalPopup.h"
#include "DB/ImageInfo.h"
#include <ktrader.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <kservice.h>
#include <kurl.h>
#include <krun.h>
#include <klocale.h>
#include <kfileitem.h>

void MainWindow::ExternalPopup::populate( DB::ImageInfoPtr current, const QStringList& imageList )
{
    _list = imageList;
    _currentInfo = current;
    clear();
    _indexOfFirstSelectionForMultipleImages = -1;

    QStringList list = QStringList() << i18n("Current Item") << i18n("All Selected Items");
    for ( int which = 0; which < 2; ++which ) {
        if ( which == 0 && !current )
            continue;

        QLabel* label = new QLabel( list[which], this );
        label->setAlignment( Qt::AlignHCenter );
        label->setLineWidth(2);
        label->setFrameStyle( QFrame::Raised | QFrame::StyledPanel );

        bool multiple = (_list.count() > 1);
        bool enabled = (which == 0 && _currentInfo ) || (which == 1 && multiple);

        int id = insertItem( label );
        setItemEnabled( id, enabled );

        OfferType offers;
        if ( which == 0 )
            offers = appInfos( QStringList() << current->fileName() );

        else
            offers = appInfos( imageList );

        for ( OfferType::const_iterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt ) {
            id = insertItem( (*offerIt).second, (*offerIt).first );
            if ( _indexOfFirstSelectionForMultipleImages == -1 && which == 1 )
                _indexOfFirstSelectionForMultipleImages = indexOf(id);

            setItemEnabled( id, enabled );
        }
    }
}

void MainWindow::ExternalPopup::slotExecuteService( int id )
{
    QString name = text( id ).remove('&');
    KTrader::OfferList offers = KTrader::self()->query( *(_appToMimeTypeMap[name].begin()), QString::fromLatin1("Type == 'Application' and Name == '%1'").arg(name));
    Q_ASSERT( offers.count() == 1 );
    KService::Ptr ptr = offers.first();
    KURL::List lst;
    if ( indexOf(id) >= _indexOfFirstSelectionForMultipleImages ) {
        for( QStringList::Iterator it = _list.begin(); it != _list.end(); ++it ) {
            if ( _appToMimeTypeMap[name].contains( mimeType(*it) ) )
                lst.append( KURL::fromPathOrURL(*it) );
        }
    }
    else {
        lst.append( KURL::fromPathOrURL(_currentInfo->fileName()));
    }

    KRun::run(*ptr, lst);
}

MainWindow::ExternalPopup::ExternalPopup( QWidget* parent, const char* name )
    :QPopupMenu( parent, name )
{
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotExecuteService( int ) ) );
}

QString MainWindow::ExternalPopup::mimeType( const QString& file )
{
    return KFileItem( KFileItem::Unknown, KFileItem::Unknown, KURL(file) ).mimetype();
}

Utilities::StringSet MainWindow::ExternalPopup::mimeTypes( const QStringList& files )
{
    StringSet res;
    for( QStringList::ConstIterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
        res.insert( mimeType( *fileIt ) );
    }
    return res;
}

MainWindow::OfferType MainWindow::ExternalPopup::appInfos(const QStringList& files )
{
    StringSet types = mimeTypes( files );
    OfferType res;
    for ( StringSet::const_iterator mimeTypeIt = types.begin(); mimeTypeIt != types.end(); ++mimeTypeIt ) {
        KTrader::OfferList offers = KTrader::self()->query( *mimeTypeIt, QString::fromLatin1("Type == 'Application'"));
        for(KTrader::OfferList::Iterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt) {
            res.insert( qMakePair( (*offerIt)->name(), (*offerIt)->pixmap(KIcon::Toolbar) ) );
            _appToMimeTypeMap[(*offerIt)->name().remove('&')].insert( *mimeTypeIt );
        }
    }
    return res;
}

bool operator<( const QPair<QString,QPixmap>& a, const QPair<QString,QPixmap>& b )
{
    return a.first < b.first;
}


#include "ExternalPopup.moc"
