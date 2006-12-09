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

#include "InvalidDateFinder.h"
#include <qlayout.h>
#include <qradiobutton.h>
#include <klocale.h>
#include <qvbuttongroup.h>
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include "DB/ImageDate.h"
#include "DB/FileInfo.h"
#include "MainWindow/Window.h"
#include "kprogress.h"
#include <qapplication.h>
#include <qeventloop.h>
#include "Utilities/ShowBusyCursor.h"
#include <qtextedit.h>

using namespace MainWindow;

InvalidDateFinder::InvalidDateFinder( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Search for Images and Videos with Missing Dates" ), Cancel | Ok, Ok, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    QVButtonGroup* grp = new QVButtonGroup( i18n("Which Images and Videos to Display"), top, "grp" );
    lay1->addWidget( grp );

    _dateNotTime = new QRadioButton( i18n( "Search for images and videos with a valid date but an invalid time stamp"), grp );
    _missingDate = new QRadioButton( i18n( "Search for images and videos missing date and time" ), grp );
    _partialDate = new QRadioButton( i18n( "Search for images and videos with only partial dates (like 1971 vs. 11/7-1971)"), grp );
    _dateNotTime->setChecked( true );
}

void InvalidDateFinder::slotOk()
{
    Utilities::ShowBusyCursor dummy;

    // create the info dialog
    KDialogBase* info = new KDialogBase(  Plain, i18n("Image Info" ), Ok, Ok, 0, "infobox", false );
    QWidget* top = info->plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );
    QTextEdit* edit = new QTextEdit( top );
    lay1->addWidget( edit );
    edit->setText( i18n("<h1>Here you may see the date changes for the displayed items.</h1>") );

    // Now search for the images.
    QStringList list = DB::ImageDB::instance()->images();
    QStringList toBeShown;
    KProgressDialog dialog( 0, "progress dialog", i18n("Reading file properties"),
                            i18n("Reading File Properties"), true );
    dialog.progressBar()->setTotalSteps( list.count() );
    dialog.progressBar()->setProgress(0);
    int progress = 0;

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info(*it);
        dialog.progressBar()->setProgress( ++progress );
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );
        if ( dialog.wasCancelled() )
            break;

        DB::ImageDate date = info->date();
        bool show = false;
        if ( _dateNotTime->isChecked() ) {
            DB::FileInfo fi = DB::FileInfo::read( info->fileName() );
            if ( fi.dateTime().date() == date.start().date() )
                show = ( fi.dateTime().time() != date.start().time() );
            if ( show ) {
                edit->append( QString::fromLatin1("%1:<br>existing = %2 %3<br>new..... = %4" )
                              .arg(info->fileName()).arg(date.start().toString())
                              .arg(date.start().time().toString())
                              .arg(fi.dateTime().toString()) );
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

    Window::theMainWindow()->showThumbNails( toBeShown );
    KDialogBase::slotOk();
}

#include "InvalidDateFinder.moc"
