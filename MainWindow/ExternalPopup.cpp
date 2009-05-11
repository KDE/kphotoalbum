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

#include "ExternalPopup.h"
#include "DB/ImageInfo.h"
#include <ktrader.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <QPixmap>
#include <kservice.h>
#include <kurl.h>
#include <krun.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <KMimeTypeTrader>
#include <KIcon>
#include "Window.h"

void MainWindow::ExternalPopup::populate( DB::ImageInfoPtr current, const QStringList& imageList )
{
    _list = imageList;
    _currentInfo = current;
    clear();

    QStringList list = QStringList() << i18n("Current Item") << i18n("All Selected Items");
    for ( int which = 0; which < 2; ++which ) {
        if ( which == 0 && !current )
            continue;

        const bool multiple = (_list.count() > 1);
        const bool enabled = (which == 0 && _currentInfo ) || (which == 1 && multiple);

        // Title
        QAction* action = addAction( list[which] );
        QFont fnt = font();
        fnt.setPointSize( static_cast<int>(fnt.pointSize()*1.5));
        fnt.setBold(true);
        action->setFont( fnt );
        action->setData( -1 );
        action->setEnabled( enabled );

        // Fetch set of offers
        OfferType offers;
        if ( which == 0 )
            offers = appInfos( QStringList() << current->fileName(DB::AbsolutePath) );
        else
            offers = appInfos( imageList );

        for ( OfferType::const_iterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt ) {
            QAction* action = addAction( (*offerIt).first );
            action->setObjectName( (*offerIt).first ); // Notice this is needed to find the application later!
            action->setIcon( KIcon((*offerIt).second) );
            action->setData( which );
            action->setEnabled( enabled );
        }
    }
}

void MainWindow::ExternalPopup::slotExecuteService( QAction* action )
{
    QString name = action->objectName();
    const StringSet apps =_appToMimeTypeMap[name];
    KService::List offers = KMimeTypeTrader::self()->query( *(apps.begin()), QString::fromLatin1("Application"),
                                                            QString::fromLatin1("Name == '%1'").arg(name));
    Q_ASSERT( offers.count() >= 1 );
    KService::Ptr ptr = offers.first();
    KUrl::List lst;
    if ( action->data() == 1 ) {
        for( QStringList::Iterator it = _list.begin(); it != _list.end(); ++it ) {
            if ( _appToMimeTypeMap[name].contains( mimeType(*it) ) )
                lst.append( KUrl(*it) );
        }
    }
    else {
        lst.append( KUrl(_currentInfo->fileName(DB::AbsolutePath)));
    }

    KRun::run(*ptr, lst, MainWindow::Window::theMainWindow() );
}

MainWindow::ExternalPopup::ExternalPopup( QWidget* parent )
    :QMenu( parent )
{
    setTitle( i18n("Invoke External Program") );
    connect( this, SIGNAL( triggered( QAction* ) ), this, SLOT( slotExecuteService( QAction* ) ) );
}

QString MainWindow::ExternalPopup::mimeType( const QString& file )
{
    return KFileItem( KFileItem::Unknown, KFileItem::Unknown, KUrl(file) ).mimetype();
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
        KService::List offers = KMimeTypeTrader::self()->query( *mimeTypeIt, QLatin1String( "Application" ));
        for(KService::List::Iterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt) {
            res.insert( qMakePair( (*offerIt)->name(), (*offerIt)->icon() ) );
            _appToMimeTypeMap[(*offerIt)->name()].insert( *mimeTypeIt );
        }
    }
    return res;
}

bool operator<( const QPair<QString,QPixmap>& a, const QPair<QString,QPixmap>& b )
{
    return a.first < b.first;
}


#include "ExternalPopup.moc"
