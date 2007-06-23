/* Copyright (C) 2007 Jan Kundrát <jkt@gentoo.org>

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
#include <klocale.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmap.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include "Exif/SyncWidget.h"

namespace Exif {

SyncWidget::SyncWidget( const QString& title, QWidget* parent, const QValueList<Syncable::Kind>& items, const char* name )
    :QHBox( parent, name )
{
    QMap<Syncable::Kind,QString> _fieldName, _visibleName, _whatsThis;
    QMap<Syncable::Kind,Syncable::Header> _header;
    Syncable::createTables( _fieldName, _visibleName, _whatsThis, _header);

    setSpacing( 6 );
    _list = new QListView( this, name );
    _list->setSorting( -1 );
    _list->addColumn( title );

    // records in QListView has to be created in reverse order...
    QValueList<Syncable::Kind>::const_iterator it = items.end();
    if ( it != items.begin() )
        do {
            --it;
            if ( _header.contains( *it ) ) {
                _items.append( *it );
                new QListViewItem( _list, _visibleName[*it], QString::number( static_cast<int>( *it ) ) );
            }
        } while ( it != items.begin() );

    QVBox* vbox = new QVBox( this );
    QWidget* spacer = new QWidget( vbox );
    _upBut = new QPushButton( vbox );
    _upBut->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "up" ), KIcon::Desktop, 22 ) );
    _downBut = new QPushButton( vbox );
    _downBut->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "down" ), KIcon::Desktop, 22 ) );
    spacer = new QWidget( vbox );

    slotHandleDisabling();

    connect( _upBut, SIGNAL( clicked() ), this, SLOT( slotMoveSelectedUp() ) );
    connect( _downBut, SIGNAL( clicked() ), this, SLOT( slotMoveSelectedDown() ) );
    connect( _list, SIGNAL( selectionChanged() ), this, SLOT( slotHandleDisabling() ) );
}

void SyncWidget::slotMoveSelectedDown()
{
    QListViewItem* item = _list->selectedItem();
    if ( !item )
        return;
    QListViewItem* other = item->itemBelow();
    if ( other ) {
        Syncable::Kind kind = static_cast<Syncable::Kind>( item->text( 1 ).toInt() );
        QValueList<Syncable::Kind>::iterator current, next;
        next = current = _items.find( kind );
        Q_ASSERT( current != _items.end() );
        --next;
        *current = *next;
        *next = kind;

        item->moveItem( other );

        slotHandleDisabling();
    }
}

void SyncWidget::slotMoveSelectedUp() {
    QListViewItem* item = _list->selectedItem();
    if ( !item )
        return;
    QListViewItem* other = item->itemAbove();
    if ( other ) {
        // remember, _items are reversed
        Syncable::Kind kind = static_cast<Syncable::Kind>( item->text( 1 ).toInt() );
        QValueList<Syncable::Kind>::iterator current, next;
        next = current = _items.find( kind );
        ++next;
        Q_ASSERT( ( current != _items.end() ) && ( next != _items.end() ) );
        *current = *next;
        *next = kind;

        other->moveItem( item );

        slotHandleDisabling();
    }
}

void SyncWidget::slotHandleDisabling()
{
    _upBut->setDisabled( true );
    _downBut->setDisabled( true );

    QListViewItem* item = _list->selectedItem();
    if ( !item )
        return;
    QListViewItem* other = item->itemAbove();
    if ( other )
        _upBut->setEnabled( true );
    other = item->itemBelow();
    if ( other )
        _downBut->setEnabled( true );
}


QValueList<Syncable::Kind> SyncWidget::items() const
{
    // _items are reversed, so we have to reverse them again here
    QValueList<Syncable::Kind> items;
    QValueList<Syncable::Kind>::const_iterator it = _items.end();
    if ( it != _items.begin() )
        do {
            --it;
            items.append( *it );
        } while ( it != _items.begin() );
    return items;
}

namespace Syncable {

void createTables( QMap<Kind,QString>& _fieldName, QMap<Kind,QString>& _visibleName, QMap<Kind,QString>& _whatsThis, QMap<Kind,Header>& _header)
{
#define I(X,HEADER,FIELD,VISIBLE,WHATS) \
     _header[X] = HEADER; \
     _fieldName[X] = #FIELD; \
     _visibleName[X] = VISIBLE ? i18n(VISIBLE) : QString::fromAscii(#FIELD); \
     _whatsThis[X] = WHATS ? i18n(WHATS) : QString::null;

    I(STOP, NONE, NONE, "-- stop --", 0);
    I(JPEG_COMMENT, JPEG, comment, "JPEG Comment", 0);
    I(EXIF_ORIENTATION, EXIF, Exif.Image.Orientation, 0, 0);
    I(EXIF_DESCRIPTION, EXIF, Exif.Image.ImageDescription, 0, 0);
    I(EXIF_USER_COMMENT, EXIF, Exif.Photo.UserComment, 0, 0);
    I(IPTC_CAPTION, IPTC, Iptc.Application2.Caption, 0, 0);
    I(IPTC_HEADLINE, IPTC, Iptc.Application2.Headline, 0, 0);

#undef I
}

}
}

#include "SyncWidget.moc"
