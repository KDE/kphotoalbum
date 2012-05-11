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

#include "ExternalPopup.h"
#include "DB/ImageInfo.h"
#include <ktrader.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <QPixmap>
#include <QFile>
#include <kservice.h>
#include <kurl.h>
#include <krun.h>
#include <kshell.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kdialog.h>
#include <kdebug.h>
#include <KMimeTypeTrader>
#include <KIcon>
#include "Window.h"
#include "RunDialog.h"
#include <DB/FileNameList.h>

void MainWindow::ExternalPopup::populate( DB::ImageInfoPtr current, const DB::FileNameList& imageList )
{
    _list = imageList;
    _currentInfo = current;
    clear();
    QAction *action;

    QStringList list = QStringList() << i18n("Current Item") << i18n("All Selected Items") << i18n("Copy and Open");
    for ( int which = 0; which < 3; ++which ) {
        if ( which == 0 && !current )
            continue;

        const bool multiple = (_list.count() > 1);
        const bool enabled = (which != 1 && _currentInfo ) || (which == 1 && multiple);

        // Submenu
        QMenu *submenu = addMenu( list[which] );
        submenu->setEnabled(enabled);

        // Fetch set of offers
        OfferType offers;
        if ( which == 0 )
            offers = appInfos( DB::FileNameList() << current->fileName() );
        else
            offers = appInfos( imageList );

        for ( OfferType::const_iterator offerIt = offers.begin(); offerIt != offers.end(); ++offerIt ) {
            action = submenu->addAction( (*offerIt).first );
            action->setObjectName( (*offerIt).first ); // Notice this is needed to find the application later!
            action->setIcon( KIcon((*offerIt).second) );
            action->setData( which );
            action->setEnabled( enabled );
        }

        // A personal command
        action = submenu->addAction( i18n("Open With...") );
        action->setObjectName( i18n("Open With...") ); // Notice this is needed to find the application later!
        // XXX: action->setIcon( KIcon((*offerIt).second) );
        action->setData( which );
        action->setEnabled( enabled );

        // A personal command
        // XXX: see kdialog.h for simple usage
        action = submenu->addAction( i18n("Your Command Line") );
        action->setObjectName( i18n("Your Command Line") ); // Notice this is needed to find the application later!
        // XXX: action->setIcon( KIcon((*offerIt).second) );
        action->setData( which );
        action->setEnabled( enabled );
    }
}

void MainWindow::ExternalPopup::slotExecuteService( QAction* action )
{
    QString name = action->objectName();
    const StringSet apps =_appToMimeTypeMap[name];

    // get the list of arguments
    KUrl::List lst;

    if ( action->data() == -1 )
    {
	return;  //user clicked the title entry. (i.e: "All Selected Items")
    } else if ( action->data() == 1 ) {
        for( DB::FileNameList::Iterator it = _list.begin(); it != _list.end(); ++it ) {
            if ( _appToMimeTypeMap[name].contains( mimeType(*it) ) )
                lst.append( KUrl((*it).absolute()) );
        }
    } else if (action->data() == 2) {
        QString origFile = _currentInfo->fileName().absolute();
        QString newFile = origFile;

        QString origRegexpString =
            Settings::SettingsData::instance()->copyFileComponent();
        QRegExp origRegexp =
            QRegExp(origRegexpString);
        QString copyFileReplacement =
            Settings::SettingsData::instance()->copyFileReplacementComponent();

        if (origRegexpString.length() > 0) {
            newFile.replace(origRegexp, copyFileReplacement);
            QFile::copy(origFile, newFile);
            lst.append( newFile );
        } else {
            qWarning("No settings were appropriate for modifying the file name (you must fill in the regexp field; Opening the original instead");
            lst.append( origFile );
        }

    } else {
        lst.append( KUrl(_currentInfo->fileName().absolute()));
    }


    // get the program to run

    // check for the special entry for self-defined
    if (name == i18n("Your Command Line")) {

        static RunDialog* dialog = new RunDialog(MainWindow::Window::theMainWindow());
        dialog->setImageList(_list);
        dialog->show();

        return;
    }

    // check for the special entry for self-defined
    if (name == i18n("Open With...")) {
        KRun::displayOpenWithDialog(lst, MainWindow::Window::theMainWindow());
        return;
    }


    KService::List offers = KMimeTypeTrader::self()->query( *(apps.begin()), QString::fromLatin1("Application"),
                                                            QString::fromLatin1("Name == '%1'").arg(name));
    Q_ASSERT( offers.count() >= 1 );
    KService::Ptr ptr = offers.first();
    KRun::run(*ptr, lst, MainWindow::Window::theMainWindow() );
}

MainWindow::ExternalPopup::ExternalPopup( QWidget* parent )
    :QMenu( parent )
{
    setTitle( i18n("Invoke External Program") );
    connect( this, SIGNAL( triggered( QAction* ) ), this, SLOT( slotExecuteService( QAction* ) ) );
}

QString MainWindow::ExternalPopup::mimeType( const DB::FileName& file )
{
    return KMimeType::findByPath(file.absolute(), 0, true)->name();
}

Utilities::StringSet MainWindow::ExternalPopup::mimeTypes( const DB::FileNameList& files )
{
    StringSet res;
    StringSet extensions;
    for( DB::FileNameList::ConstIterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
       const DB::FileName baseFileName = *fileIt;
       const int extStart = baseFileName.relative().lastIndexOf(QChar::fromLatin1('.'));
       const QString ext = baseFileName.relative().mid(extStart);
       if (! extensions.contains(ext)) {
           res.insert( mimeType( *fileIt ) );
           extensions.insert( ext );
       }
    }
    return res;
}

MainWindow::OfferType MainWindow::ExternalPopup::appInfos(const DB::FileNameList& files )
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
