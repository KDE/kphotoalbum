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
        return new ContentFolderAction( QString::null, QString::null, info, _browser );
    }
    else
        return 0;
}

DateSearchDialog::DateSearchDialog( QWidget* parent, const char* name )
    :KDialogBase( KDialogBase::Plain, i18n("Date search"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, parent, name ), _toChanged( false )
{
    QWidget* top = plainPage();
    QHBoxLayout* lay1 = new QHBoxLayout( top, 6 );

    QVBoxLayout* lay2 = new QVBoxLayout( lay1, 6 );

    // PENDING(blackie) When done with message freze, join the two strings below
    QLabel* label = new QLabel( QString::fromLatin1( "%1:").arg(i18n("From") ), top );
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
    // PENDING(blackie) When done with message freze, join the two strings below
    label = new QLabel( QString::fromLatin1( "%1:" ).arg( i18n("To") ), top );
    increaseFont( label, 2 );

    lay3->addWidget( label );
    _to = new KDatePicker( top );
    lay3->addWidget( _to );

    _from->setDate( QDate( QDate::currentDate().year(), 1, 1 ) );
    _to->setDate( QDate( QDate::currentDate().year()+1, 1, 1 ) );
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
    if ( !_toChanged )
        _to->setDate( date );
}

void DateSearchDialog::toDateChanged()
{
    _toChanged = true;
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

#include "datefolder.moc"
