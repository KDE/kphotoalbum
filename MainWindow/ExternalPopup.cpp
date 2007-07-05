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
//Added by qt3to4:
#include <QPixmap>
#include <Q3Frame>
#include <Q3PopupMenu>
#include <kservice.h>
#include <kurl.h>
#include <krun.h>
#include <klocale.h>
#include <kfileitem.h>

void MainWindow::ExternalPopup::populate( DB::ImageInfoPtr current, const QStringList& imageList )
{
#ifdef TEMPORARILY_REMOVED
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
        label->setFrameStyle( Q3Frame::Raised | Q3Frame::StyledPanel );

        bool multiple = (_list.count() > 1);
        bool enabled = (which == 0 && _currentInfo ) || (which == 1 && multiple);

        int id = insertItem( label );
        setItemEnabled( id, enabled );

        OfferType offers;
        if ( which == 0 )
            offers = appInfos( QStringList() << current->fileName() );

        else
            offers = appInfos( imageList );

        for ( OfferType::ConstIterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt ) {
            id = insertItem( (*offerIt).second, (*offerIt).first );
            if ( _indexOfFirstSelectionForMultipleImages == -1 && which == 1 )
                _indexOfFirstSelectionForMultipleImages = indexOf(id);

            setItemEnabled( id, enabled );
        }
    }
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

void MainWindow::ExternalPopup::slotExecuteService( int id )
{
#ifdef TEMPORARILY_REMOVED
    QString name = text( id );
    KTrader::OfferList offers = KTrader::self()->query( *(_appToMimeTypeMap[name].begin()), QString::fromLatin1("Type == 'Application' and Name == '%1'").arg(name));
    Q_ASSERT( offers.count() == 1 );
    KService::Ptr ptr = offers.first();
    KUrl::List lst;
    if ( indexOf(id) >= _indexOfFirstSelectionForMultipleImages ) {
        for( QStringList::Iterator it = _list.begin(); it != _list.end(); ++it ) {
            if ( _appToMimeTypeMap[name].contains( mimeType(*it) ) )
                lst.append( KUrl::fromPathOrUrl(*it) );
        }
    }
    else {
        lst.append( KUrl::fromPathOrUrl(_currentInfo->fileName()));
    }

    KRun::run(*ptr, lst);
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

MainWindow::ExternalPopup::ExternalPopup( QWidget* parent, const char* name )
    :Q3PopupMenu( parent, name )
{
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotExecuteService( int ) ) );
}

QString MainWindow::ExternalPopup::mimeType( const QString& file )
{
    return KFileItem( KFileItem::Unknown, KFileItem::Unknown, KUrl(file) ).mimetype();
}

StringSet MainWindow::ExternalPopup::mimeTypes( const QStringList& files )
{
    StringSet res;
    for( QStringList::ConstIterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
        res.insert( mimeType( *fileIt ) );
    }
    return res;
}

MainWindow::OfferType MainWindow::ExternalPopup::appInfos(const QStringList& files )
{
#ifdef TEMPORARILY_REMOVED
    StringSet types = mimeTypes( files );
    OfferType res;
    for ( StringSet::ConstIterator mimeTypeIt = types.begin(); mimeTypeIt != types.end(); ++mimeTypeIt ) {
        KTrader::OfferList offers = KTrader::self()->query( *mimeTypeIt, QString::fromLatin1("Type == 'Application'"));
        for(KTrader::OfferList::Iterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt) {
            res.insert( qMakePair( (*offerIt)->name(), (*offerIt)->pixmap(K3Icon::Toolbar) ) );
            _appToMimeTypeMap[(*offerIt)->name()].insert( *mimeTypeIt );
        }
    }
    return res;
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

bool operator<( const QPair<QString,QPixmap>& a, const QPair<QString,QPixmap>& b )
{
    return a.first < b.first;
}


#include "ExternalPopup.moc"
