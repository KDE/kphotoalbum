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
#include <qwhatsthis.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include "Exif/SyncWidget.h"

/**
 * SyncWidget is a ListView-like container with two buttons for moving selected
 * item up and down in a hierarchy. It isn't important what items are selected,
 * but the order in which they appear.
 * Implementation of moving is pretty simple and suitable only for a relatively
 * small amount of available items. Typical usage doesn't contain more than
 * twenty of those, so it's usable without any troubles.
 */

namespace Exif {

/*
 * Constructor. Argument "items" is a list of supported item types.
 */
SyncWidget::SyncWidget( const QString& title, QWidget* parent, const QValueList<Syncable::Kind>& items, const char* name )
    :QHBox( parent, name )
{
    QMap<Syncable::Kind,QString> _fieldName, _visibleName;
    QMap<Syncable::Kind,Syncable::Header> _header;
    Syncable::fillTranslationTables( _fieldName, _visibleName, _header);

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

    QWhatsThis::add( this, i18n("<p>Use the \"Up\" and \"Down\" buttons to change the order of various fields. "
                "Items are processed from the top of this list and the iteration stops when it hits the "
                "<code>-- stop --</code> field.</p>") );

    slotHandleDisabling();

    connect( _upBut, SIGNAL( clicked() ), this, SLOT( slotMoveSelectedUp() ) );
    connect( _downBut, SIGNAL( clicked() ), this, SLOT( slotMoveSelectedDown() ) );
    connect( _list, SIGNAL( selectionChanged() ), this, SLOT( slotHandleDisabling() ) );
}

/*
 * Move currently selected item one step down
 */
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

/* 
 * Move currently selected item one step up
 */
void SyncWidget::slotMoveSelectedUp()
{
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

/*
 * Enable/Disable up and down buttons based on whether currently selected item
 * is movable
 */
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

/*
 * Find out order of preferences for various fields
 */
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

/*
 * Update current preferences so that first items are those passed to this
 * function *and* set in constructor like "supported" ones. The order of items
 * in the "items" variable is the intuitive one, ie. the preferred items should
 * be placed near the begin of the list.
 */
void SyncWidget::updatePreferred( const QValueList<Syncable::Kind>& items )
{
    QValueList<Syncable::Kind> copyOfItems( _items );
    _list->clear();
    _items.clear();

    QMap<Syncable::Kind,QString> _fieldName, _visibleName;
    QMap<Syncable::Kind,Syncable::Header> _header;
    Syncable::fillTranslationTables( _fieldName, _visibleName, _header);

    /*
     * Records in QListView has to be created in reversed order, so we take a
     * bit hackish way.
     *
     * Here we create all items that user isn't interested in (and in correct
     * order)
     */
    for (QValueList<Syncable::Kind>::const_iterator it = copyOfItems.begin();
            it != copyOfItems.end(); ++it )
        if ( !items.contains( *it ) ) {
            _items.append( *it );
            new QListViewItem( _list, _visibleName[*it], QString::number( static_cast<int>( *it ) ) );
        }
    /*
     * Now we go through the user-supplied fields from the end, adding items on
     * the fly if they are allowed here
     */
    QValueList<Syncable::Kind>::const_iterator it = items.end();
    if ( it != items.begin() )
        do {
            --it;
            if ( copyOfItems.contains( *it ) && !_items.contains( *it ) ) {
                _items.append( *it );
                new QListViewItem( _list, _visibleName[*it], QString::number( static_cast<int>( *it ) ) );
            }
        } while ( it != items.begin() );
    slotHandleDisabling();
}

}

#include "SyncWidget.moc"
