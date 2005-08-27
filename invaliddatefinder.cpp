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

#include "invaliddatefinder.h"
#include <qlayout.h>
#include <qradiobutton.h>
#include <klocale.h>
#include <qvbuttongroup.h>
#include "imageinfo.h"
#include "imagedb.h"
#include "imagedate.h"
#include "fileinfo.h"
#include "mainview.h"
#include "kprogress.h"
#include <qapplication.h>
#include <qeventloop.h>
#include "showbusycursor.h"
#include <qtextedit.h>
#include <kdebug.h>

InvalidDateFinder::InvalidDateFinder( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Search for Images with Missing Dates" ), Cancel | Ok, Ok, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    QVButtonGroup* grp = new QVButtonGroup( i18n("Which Images to Display"), top, "grp" );
    lay1->addWidget( grp );

    _dateNotTime = new QRadioButton( i18n( "Search for images with a valid date but an invalid time stamp"), grp );
    _missingDate = new QRadioButton( i18n( "Search for images missing date and time" ), grp );
    _partialDate = new QRadioButton( i18n( "Search for images with only partial dates (like 1971 vs. 11/7-1971)"), grp );
    _dateNotTime->setChecked( true );
}

void InvalidDateFinder::slotOk()
{
    ShowBusyCursor dummy;

    // create the info dialog
    KDialogBase* info = new KDialogBase(  Plain, i18n("Image Info" ), Ok, Ok, 0, "infobox", false );
    QWidget* top = info->plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );
    QTextEdit* edit = new QTextEdit( top );
    lay1->addWidget( edit );
    edit->setText( i18n("<h1>Here you may see the image date changes for the displayed images.</h1>") );

    // Now search for the images.
    QStringList list = ImageDB::instance()->images();
    QStringList toBeShown;
    KProgressDialog dialog( 0, "progress dialog", i18n("Reading file properties"),
                            i18n("Reading File Properties"), true );
    dialog.progressBar()->setTotalSteps( list.count() );
    dialog.progressBar()->setProgress(0);
    int progress = 0;

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        ImageInfoPtr info = ImageDB::instance()->info(*it);
        dialog.progressBar()->setProgress( ++progress );
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );
        if ( dialog.wasCancelled() )
            break;

        ImageDate date = info->date();
        bool show = false;
        if ( _dateNotTime->isChecked() ) {
            FileInfo fi = FileInfo::read( info->fileName() );
            if ( fi.date() == date.start().date() )
                show = ( fi.time() != date.start().time() );
            if ( show ) {
                edit->append( QString::fromLatin1("%1:<br>existing = %2 %3<br>new..... = %4 %5" )
                              .arg(info->fileName()).arg(date.start().toString())
                              .arg(date.start().time().toString())
                              .arg(fi.date().toString()).arg( fi.time().toString() ) );
            }
        }
        else if ( _missingDate->isChecked() ) {
            show = !date.start().isValid();
        }
        else if ( _partialDate->isChecked() ) {
            show = ( date.start() != date.end() );
        }

        if ( show )
            toBeShown.append( *it );
    }

    if ( _dateNotTime->isChecked() ) {
        info->resize( 800, 600 );
        edit->setCursorPosition( 0,0 );
        edit->setReadOnly( true );
        QFont f = edit->font();
        f.setFamily( QString::fromLatin1( "fixed" ) );
        edit->setFont( f );
        info->show();
    }
    else
        delete info;

    MainView::theMainView()->showThumbNails( toBeShown );
    KDialogBase::slotOk();
}

#include "invaliddatefinder.moc"
