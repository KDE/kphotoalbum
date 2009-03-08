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

#include "InvalidDateFinder.h"
#include <qlayout.h>
#include <qradiobutton.h>
#include <klocale.h>
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include "DB/ImageDate.h"
#include "DB/FileInfo.h"
#include "DB/ResultId.h"
#include "MainWindow/Window.h"
#include <qapplication.h>
#include <qeventloop.h>
#include "Utilities/ShowBusyCursor.h"
#include <QGroupBox>
#include <QTextEdit>
#include <KProgressDialog>
#include <kdebug.h>

using namespace MainWindow;

InvalidDateFinder::InvalidDateFinder( QWidget* parent )
    :KDialog( parent )
{
    setWindowTitle( i18n("Search for Images and Videos with Missing Dates" ) );
    setButtons( Cancel | Ok );

    QWidget* top = new QWidget;
    setMainWidget( top );
    QVBoxLayout* lay1 = new QVBoxLayout( top );

    QGroupBox* grp = new QGroupBox( i18n("Which Images and Videos to Display") );
    QVBoxLayout* grpLay = new QVBoxLayout( grp );
    lay1->addWidget( grp );

    _dateNotTime = new QRadioButton( i18n( "Search for images and videos with a valid date but an invalid time stamp") );
    _missingDate = new QRadioButton( i18n( "Search for images and videos missing date and time" ) );
    _partialDate = new QRadioButton( i18n( "Search for images and videos with only partial dates (like 1971 vs. 11/7-1971)") );
    _dateNotTime->setChecked( true );

    grpLay->addWidget( _dateNotTime );
    grpLay->addWidget( _missingDate );
    grpLay->addWidget( _partialDate );
}

void InvalidDateFinder::accept()
{
    KDialog::accept();
    Utilities::ShowBusyCursor dummy;

    // create the info dialog
    KDialog* info = new KDialog;
    info->setWindowTitle( i18n("Image Info" ) );
    info->setButtons( Ok );

    QWidget* top = new QWidget;
    info->setMainWidget( top );

    QVBoxLayout* lay1 = new QVBoxLayout( top );
    QTextEdit* edit = new QTextEdit( top );
    lay1->addWidget( edit );
    edit->setText( i18n("<h1>Here you may see the date changes for the displayed items.</h1>") );

    // Now search for the images.
    DB::Result list = DB::ImageDB::instance()->images();
    DB::Result toBeShown;
    KProgressDialog dialog( 0, i18n("Reading file properties"),
                            i18n("Reading File Properties") );
    dialog.progressBar()->setMaximum(list.size());
    dialog.progressBar()->setValue(0);
    int progress = 0;

    Q_FOREACH(DB::ResultId id, list) {
        DB::ImageInfoPtr info = id.fetchInfo();
        dialog.progressBar()->setValue( ++progress );
        qApp->processEvents( QEventLoop::AllEvents );
        if ( dialog.wasCancelled() )
            break;

        DB::ImageDate date = info->date();
        bool show = false;
        if ( _dateNotTime->isChecked() ) {
            DB::FileInfo fi = DB::FileInfo::read( info->fileName(DB::AbsolutePath), DB::EXIFMODE_DATE );
            if ( fi.dateTime().date() == date.start().date() )
                show = ( fi.dateTime().time() != date.start().time() );
            if ( show ) {
                edit->append( QString::fromLatin1("%1:<br/>existing = %2<br>new..... = %3" )
                              .arg(info->fileName(DB::AbsolutePath))
                              .arg(date.start().toString())
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
            toBeShown.append(id);
    }

    if ( _dateNotTime->isChecked() ) {
        info->resize( 800, 600 );
        edit->setReadOnly( true );
        QFont f = edit->font();
        f.setFamily( QString::fromLatin1( "fixed" ) );
        edit->setFont( f );
        info->show();
    }
    else
        delete info;

    Window::theMainWindow()->showThumbNails( toBeShown );
}

#include "InvalidDateFinder.moc"
