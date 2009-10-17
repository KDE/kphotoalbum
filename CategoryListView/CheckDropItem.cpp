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
#include "CheckDropItem.h"
#include <DB/MemberMap.h>
#include <DB/ImageDB.h>
#include "DragableListView.h"
#include "DragItemInfo.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <QDropEvent>
#include "DB/CategoryItem.h"
#include "DB/Category.h"

CategoryListView::CheckDropItem::CheckDropItem( DragableListView* parent, const QString& column1,
                                                const QString& column2 )
    : Q3CheckListItem( parent, column1, Q3CheckListItem::CheckBox ), _listView( parent )
{
    setText( 1, column2 );
}

CategoryListView::CheckDropItem::CheckDropItem( DragableListView* listView, Q3ListViewItem* parent, const QString& column1,
                                                const QString& column2 )
    : Q3CheckListItem( parent, column1, Q3CheckListItem::CheckBox ), _listView( listView )
{
    setText( 1, column2 );
}

bool CategoryListView::CheckDropItem::acceptDrop( const QMimeSource* mime ) const
{
    if ( !mime->provides( "x-kphotoalbum/x-category-drag" ) )
        return false;

    const QDropEvent* devent = dynamic_cast<const QDropEvent*>(mime);
    if ( !devent )
        return false;

    if ( devent->source() != listView() )
        return false;

    return !isSelfDrop( text(0), extractData( mime ) );
}

CategoryListView::DragItemInfoSet CategoryListView::CheckDropItem::extractData( const QMimeSource* e) const
{
    DragItemInfoSet items;
    const QByteArray data = e->encodedData( "x-kphotoalbum/x-category-drag" );
    QDataStream stream( data );
    stream >> items;

    return items;
}

void CategoryListView::CheckDropItem::dropped( QDropEvent* e )
{
    DragItemInfoSet items = extractData( e );
    const QString newParent = text(0);
    if ( !verifyDropWasIntended( newParent, items ) )
        return;

    DB::MemberMap& memberMap = DB::ImageDB::instance()->memberMap();
    memberMap.addGroup( _listView->category()->name(), newParent );

    for( DragItemInfoSet::const_iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        const QString oldParent = (*itemIt).parent();
        const QString child = (*itemIt).child();

        memberMap.addMemberToGroup( _listView->category()->name(), newParent, child );

        if ( !oldParent.isNull() )
            memberMap.removeMemberFromGroup( _listView->category()->name(), oldParent, child );
    }

    //DB::ImageDB::instance()->setMemberMap( memberMap );

    _listView->emitItemsChanged();
}

bool CategoryListView::CheckDropItem::isSelfDrop( const QString& newParent, const DragItemInfoSet& children ) const
{
    const DB::CategoryItemPtr categoryInfo = _listView->category()->itemsCategories();

    for( DragItemInfoSet::const_iterator childIt = children.begin(); childIt != children.end(); ++childIt ) {
        if ( newParent == (*childIt).child() || categoryInfo->isDescendentOf( newParent, (*childIt).child() ) )
            return true;
    }
    return false;
}

bool CategoryListView::CheckDropItem::verifyDropWasIntended( const QString& parent, const DragItemInfoSet& items )
{
    QStringList children;
    for( DragItemInfoSet::const_iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        children.append( (*itemIt).child() );
    }

    const QString msg =
        i18np("<p>You have just dragged an item onto another: this will make the dragged item a sub category "
              "of that onto which it was dropped. Sub categories may be used to denote facts such as 'Las Vegas is in the USA', "
              "in that example you would drag Las Vegas onto USA. When you have set up sub categories you can, for instance, "
              "see all images from the USA by simply selecting that item in the Browser.</p>"
              "<p>Was it really your intention to make \"%2\" a sub category of \"%3\"?</p>",
              "<p>You have just dragged a number of items onto another: this will make the dragged items sub categories "
              "of that onto which they were dropped. Sub categories may be used to denote facts such as 'Las Vegas and New York are in the USA' "
              "in that example you would drag 'Las Vegas' and 'New York' onto USA. When you have set up sub categories you can, for instance, "
              "see all images from the USA by simply selecting that item in the Browser.</p>"
              "<p>Was it really your intention to make \"%2\" sub categories of \"%3\"?</p>",
              children.size(), children.join( QString::fromLatin1( ", " ) ), parent );

    const int answer = KMessageBox::warningContinueCancel( 0, msg, i18n("Move Items"), KStandardGuiItem::cont(),
                                                           KStandardGuiItem::cancel(),
                                                           QString::fromLatin1( "DoYouReallyWantToMessWithMemberGroups" ) );
    return answer == KMessageBox::Continue;
}

void CategoryListView::CheckDropItem::setDNDEnabled( const bool b )
{
    setDragEnabled( b );
    setDropEnabled( b );
}

/**
 * Qt's implementation of QCheckListItem toggle between "on", "unchanged", and "off". I don't like it going to the unchanged state,
 * esp. because in most (perhaps even all) styles, it is really hard to tell the difference between "unchanged" and "on",
 * The user might therefore not really understand that when he toggles from off to unchanged, that the item does not get
 * selected in his database.
 */
void CategoryListView::CheckDropItem::activate()
{
    if ( state() == Off )
        setState( On );
    else
        setState( Off );
}

