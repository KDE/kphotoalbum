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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "datefolder.h"
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qlayout.h>
#include <kdatepicker.h>
#include <qlabel.h>
#include "contentfolder.h"
#include <qpushbutton.h>
#include <qobjectlist.h>
#include "mainview.h"
#include <kdatetbl.h>
#include <qdatetime.h>
#include "imagedb.h"
#include "imageinfo.h"
#include "imagesearchinfo.h"
#include "showbusycursor.h"

DateFolder::DateFolder( const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent )
{
    setCount( -1 );
}

QPixmap DateFolder::pixmap()
{
    KIconLoader loader;
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "korganizer" ), KIcon::Desktop, 22 );
}

QString DateFolder::text() const
{
    return i18n( "Date" );
}


FolderAction* DateFolder::action( bool /* ctrlDown */ )
{
    DateSearchDialog dialog( _browser );
    if ( dialog.exec() == QDialog::Accepted ) {
        ImageSearchInfo info( _info );
        info.setStartDate( ImageDate( dialog.fromDate() ) );
        info.setEndDate( ImageDate( dialog.toDate() ) );
	Options::instance()->setFromDate( dialog.fromDate() );
	Options::instance()->setToDate( dialog.toDate() );
        return new ContentFolderAction( QString::null, QString::null, info, _browser );
    }
    else
        return 0;
}

DateSearchDialog::DateSearchDialog( QWidget* parent, const char* name )
    :KDialogBase( KDialogBase::Plain, i18n("Date Search"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent, name ), _toChanged( false )
{
    ShowBusyCursor dummy;
    QWidget* top = plainPage();
    QHBoxLayout* lay1 = new QHBoxLayout( top, 6 );

    QVBoxLayout* lay2 = new QVBoxLayout( lay1, 6 );

    QLabel* label = new QLabel( i18n("From:"), top );
    increaseFont( label, 2 );
    lay2->addWidget( label );
    _from = new KDatePicker( top );

    lay2->addWidget( _from );

    lay1->addSpacing( 25 );
    QPushButton* copy = new QPushButton( QString::fromLatin1( ">>" ), top );
    copy->setFixedWidth( 30 );
    lay1->addWidget( copy );
    lay1->addSpacing( 25 );
    connect( copy, SIGNAL( clicked() ), this, SLOT( slotCopy() ) );

    QVBoxLayout* lay3 = new QVBoxLayout( lay1, 6 );
    label = new QLabel( i18n("To:"), top );
    increaseFont( label, 2 );

    lay3->addWidget( label );
    _to = new KDatePicker( top );
    lay3->addWidget( _to );

    _from->setDate( Options::instance()->fromDate() );
    _to->setDate( Options::instance()->toDate() );
    _prevFrom = _from->date();
    _prevTo   = _to   ->date();
    highlightPossibleDates( _from );
    highlightPossibleDates( _to   );
    connect( _from, SIGNAL( dateChanged( QDate ) ), this, SLOT( fromDateChanged( QDate ) ) );
    connect( _to, SIGNAL( dateChanged( QDate ) ), this, SLOT( toDateChanged() ) );

    disableDefaultButtons();
}

QDate DateSearchDialog::fromDate() const
{
    return _from->date();
}

QDate DateSearchDialog::toDate() const
{
    return _to->date();
}

void DateSearchDialog::fromDateChanged( QDate date )
{
    if ( (_prevFrom.year() != date.year()) || (_prevFrom.month() != date.month() ) )
	highlightPossibleDates( _from );
    if ( !_toChanged )
        _to->setDate( date );
    _prevFrom = date;
}

void DateSearchDialog::toDateChanged()
{
    QDate date = _to->date();
    if ( (_prevTo.year() != date.year()) || (_prevTo.month() != date.month() ) )
	highlightPossibleDates( _to );
    _toChanged = true;
    _prevTo = date;
}

QString DateFolder::countLabel() const
{
    return QString::null;
}

void DateSearchDialog::slotCopy()
{
    _to->setDate( _from->date() );
}

void DateSearchDialog::disableDefaultButtons()
{
    QObjectList* list = queryList( "QPushButton" );
    QObject* obj;
    for ( QObjectListIt it( *list ); (obj = it.current()) != 0; ++it ) {
        ((QPushButton*)obj)->setDefault( false );
        ((QPushButton*)obj)->setAutoDefault( false );
    }
    delete list; // delete the list, not the objects
}

void DateSearchDialog::increaseFont( QWidget* widget, int factor )
{
    QFont font = widget->font();
    font.setPointSize( font.pointSize() * factor );
    widget->setFont( font );
}

void DateSearchDialog::highlightPossibleDates( KDatePicker* picker)
{
    ShowBusyCursor dummy;
    KDateTable * dateTable = picker->dateTable();
    ImageSearchInfo context = MainView::theMainView()->currentContext();
    ImageDate date = ImageDate( picker->date() );

    int nbDays = date.getDate().daysInMonth() ;
    bool picturesByDays[ 31 ];
    for (int i = 0 ; i < 31 ; i++ ) {
        picturesByDays[i] = false;
    }

    date.setDay( 1 );
    context.setStartDate( date );
    date.setDay( date.getDate().daysInMonth() );
    context.setEndDate( date );
    ImageInfoList list = ImageDB::instance()->images( context, TRUE );

    // Iterate through the list of images in the current context that is within the current month.
    for( ImageInfo* image = list.first(); image ; image=list.next() ) {
        if ( image->startDate().isFuzzyData() || ( !image->endDate().isNull() && image->endDate().isFuzzyData() ) )
            continue;
        QDate start = image->startDate().getDate();
        QDate end = image->endDate().getDate();

        int a,b;
        a = start.day();
        if ( date.year()  < start.year() )
            a = 1;
        else if (date.month() < start.month() )
            a=1;

        b = end.day();
        if ( end.year() < date.year()  ) //TODO: how to check there is no second date ?
            b = a;
        else if (date.year()  > end.year())
            b = nbDays;
        else if (date.month() > end.month() )
            b = nbDays;

        for (int i = a ; i <= b ; i++ )
            picturesByDays[ i-1 ] = true;
    }

    // Now highlight the images
    for( int i = 0 ; i < nbDays ; i++ ) {
        if (picturesByDays[ i ] == false)
            continue;
        date.setDay( i+1 );
        dateTable->setCustomDatePainting( date.getDate(), QColor( "red" ) );
    }
}
#include "datefolder.moc"
