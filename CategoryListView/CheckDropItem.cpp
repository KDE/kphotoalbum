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
#include "CheckDropItem.h"
#include <DB/MemberMap.h>
#include <DB/ImageDB.h>
#include "DragableTreeWidget.h"
#include "DragItemInfo.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <QDropEvent>
#include "DB/CategoryItem.h"
#include "DB/Category.h"

CategoryListView::CheckDropItem::CheckDropItem( DragableTreeWidget* parent, const QString& column1,
                                                const QString& column2 )
    : QTreeWidgetItem( parent ), m_listView( parent )
{
    setCheckState(0, Qt::Unchecked);
    setText( 0, column1 );
    setText( 1, column2 );
}

CategoryListView::CheckDropItem::CheckDropItem( DragableTreeWidget* listView, QTreeWidgetItem* parent, const QString& column1,
                                                const QString& column2 )
    : QTreeWidgetItem( parent ), m_listView( listView )
{
    setCheckState(0, Qt::Unchecked);
    setText( 0, column1 );
    setText( 1, column2 );
}

CategoryListView::DragItemInfoSet CategoryListView::CheckDropItem::extractData( const QMimeData* data) const
{
    DragItemInfoSet items;
    QByteArray array = data->data(QString::fromUtf8("x-kphotoalbum/x-categorydrag"));
    QDataStream stream( array );
    stream >> items;

    return items;
}

bool CategoryListView::CheckDropItem::dataDropped( const QMimeData* data )
{
    DragItemInfoSet items = extractData( data );
    const QString newParent = text(0);
    if ( !verifyDropWasIntended( newParent, items ) )
        return false;

    DB::MemberMap& memberMap = DB::ImageDB::instance()->memberMap();
    memberMap.addGroup( m_listView->category()->name(), newParent );

    for( DragItemInfoSet::const_iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        const QString oldParent = (*itemIt).parent();
        const QString child = (*itemIt).child();

        memberMap.addMemberToGroup( m_listView->category()->name(), newParent, child );
        memberMap.removeMemberFromGroup( m_listView->category()->name(), oldParent, child );
    }

    //DB::ImageDB::instance()->setMemberMap( memberMap );

    m_listView->emitItemsChanged();
    return true;
}

bool CategoryListView::CheckDropItem::isSelfDrop(const QMimeData *data) const
{
    const QString thisCategory = text(0);
    const DragItemInfoSet children = extractData(data);
    const DB::CategoryItemPtr categoryInfo = m_listView->category()->itemsCategories();

    for( DragItemInfoSet::const_iterator childIt = children.begin(); childIt != children.end(); ++childIt ) {
        if ( thisCategory == (*childIt).child() || categoryInfo->isDescendentOf( thisCategory, (*childIt).child() ) )
            return true;
    }
    return false;
}

void CategoryListView::CheckDropItem::setTristate(bool b)
{
    if (b)
        setFlags(flags() | Qt::ItemIsTristate );
    else
        setFlags(flags() & ~Qt::ItemIsTristate);
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

    const int answer = KMessageBox::warningContinueCancel( nullptr, msg, i18n("Move Items"), KStandardGuiItem::cont(),
                                                           KStandardGuiItem::cancel(),
                                                           QString::fromLatin1( "DoYouReallyWantToMessWithMemberGroups" ) );
    return answer == KMessageBox::Continue;
}

void CategoryListView::CheckDropItem::setDNDEnabled( const bool b )
{
    if ( b )
        setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled| flags());
    else
        setFlags(flags() & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled );
}
// vi:expandtab:tabstop=4 shiftwidth=4:
