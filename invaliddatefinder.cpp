/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
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

InvalidDateFinder::InvalidDateFinder( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Search for Images with Missing Dates" ), Cancel | Ok, Ok, parent, name )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    QVButtonGroup* grp = new QVButtonGroup( i18n("Which Iimages to display"), top, "grp" );
    lay1->addWidget( grp );

    _dateNotTime = new QRadioButton( i18n( "Search for images with a valid date but invalid time stamp"), grp );
    _missingDate = new QRadioButton( i18n( "Search for images missing date and time" ), grp );
    _partialDate = new QRadioButton( i18n( "Search for images with only partial dates (like 1971 vs. 11/7-1971)"), grp );
    _dateNotTime->setChecked( true );
}

void InvalidDateFinder::slotOk()
{
    ShowBusyCursor dummy;
    ImageInfoList list = ImageDB::instance()->images();
    KProgressDialog dialog( 0, "progress dialog", i18n("Reading file properties"),
                            i18n("Reading file properties"), true );
    dialog.progressBar()->setTotalSteps( list.count() );
    dialog.progressBar()->setProgress(0);
    int progress = 0;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        dialog.progressBar()->setProgress( ++progress );
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );
        if ( dialog.wasCancelled() )
            break;

        ImageDate date = (*it)->startDate();
        bool show = false;
        if ( _dateNotTime->isChecked() ) {
            FileInfo fi = FileInfo::read( (*it)->fileName() );
            if ( fi.date() == (*it)->startDate().getDate() )
                show = ( fi.time() != (*it)->startDate().getTime() );
            if ( show ) {
                qDebug("%s: %s<->%s", (*it)->fileName().latin1(),
                       fi.time().toString().latin1(), (*it)->startDate().getTime().toString().latin1());
            }
        }
        else if ( _missingDate->isChecked() ) {
            show = ( date.year() == 0 ); // For things to work, we really need a year.
        }
        else if ( _partialDate->isChecked() ) {
            show = (date.year() == 0 || date.month() == 0 || date.day() == 0);
        }

        (*it)->setVisible( show );
    }
    KDialogBase::slotOk();
}
