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
#include "RangeWidget.h"
#include <qlabel.h>
#include <KComboBox>
#include <q3grid.h>

Exif::RangeWidget::RangeWidget( const QString& text, const QString& searchTag, const ValueList& list, Q3Grid* parent )
    : QObject( parent ),_searchTag ( searchTag ), _list( list )
{
	// widget layout: <title text> <from_value> "to" <to_value>
	// register title text:
    new QLabel( text, parent );
	// register from-field:
    _from = new KComboBox( parent );
	// register filler between from- and to-field:
    new QLabel( QString::fromLatin1( "to" ), parent );
	// register to-field:
    _to = new KComboBox( parent );

    Q_ASSERT( list.count() > 2 );
    ValueList::ConstIterator it = list.begin();
    _from->addItem( QString::fromLatin1( "< %1" ).arg( (*it).text ) );

    for( ; it != list.end(); ++it ) {
        _from->addItem( (*it).text );
    }

    _from->addItem( QString::fromLatin1( "> %1" ).arg( list.last().text ) );
    slotUpdateTo( 0 );
    _to->setCurrentIndex( _to->count()-1);// set range to be min->max

    connect( _from, SIGNAL( activated( int ) ), this, SLOT( slotUpdateTo( int ) ) );
}

void Exif::RangeWidget::slotUpdateTo( int fromIndex )
{
    _to->clear();

    if ( fromIndex == 0 )
        _to->addItem( QString::fromLatin1( "< %1" ).arg( _list.first().text ) );
    else
        fromIndex--;

    for ( int i = fromIndex; i < _list.count(); ++i ) {
        _to->addItem( _list[i].text );
    }
    _to->addItem( QString::fromLatin1( "> %1" ).arg( _list.last().text ) );
}

Exif::SearchInfo::Range Exif::RangeWidget::range() const
{
    SearchInfo::Range result( _searchTag );
    result.min = _list.first().value;
    result.max = _list.last().value;
    if ( _from->currentIndex() == 0 )
        result.isLowerMin = true;
    else if ( _from->currentIndex() == _from->count() -1 )
        result.isLowerMax = true;
    else
        result.min = _list[_from->currentIndex()-1].value;


    if ( _to->currentIndex() == 0 && _from->currentIndex() == 0 )
        result.isUpperMin = true;
    else if ( _to->currentIndex() == _to->count() -1 )
        result.isUpperMax = true;
    else
        result.max =  _list[_to->currentIndex() + _from->currentIndex()-1].value;

    return result;
}

#include "RangeWidget.moc"
