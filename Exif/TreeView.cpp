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
#include "Exif/TreeView.h"
#include "Utilities/Set.h"
#include <qmap.h>
#include <qstringlist.h>
#include "Exif/Info.h"
#include <klocale.h>

using Utilities::StringSet;

Exif::TreeView::TreeView( const QString& title, QWidget* parent, const char* name )
    :QListView( parent, name )
{
    addColumn( title );
    reload();
    connect( this, SIGNAL( clicked( QListViewItem* ) ), this, SLOT( toggleChildren( QListViewItem* ) ) );
}

void Exif::TreeView::toggleChildren( QListViewItem* parent )
{
    if ( !parent )
        return;

    QCheckListItem* par = static_cast<QCheckListItem*>( parent );
    bool on = par->isOn();
    for ( QListViewItem* child = parent->firstChild(); child; child = child->nextSibling() ) {
        static_cast<QCheckListItem*>(child)->setOn( on );
        toggleChildren( child );
    }
}

StringSet Exif::TreeView::selected()
{
    StringSet result;
    for ( QListViewItemIterator it( this ); *it; ++it ) {
        if ( static_cast<QCheckListItem*>( *it )->isOn() )
            result.insert( (*it)->text( 1 ) );
    }
    return result;
}

void Exif::TreeView::setSelected( const StringSet& selected )
{
    for ( QListViewItemIterator it( this ); *it; ++it ) {
        bool on = selected.contains( (*it)->text(1) );
        static_cast<QCheckListItem*>(*it)->setOn( on );
    }
}

void Exif::TreeView::reload()
{
    clear();
    StringSet keys = Exif::Info::instance()->availableKeys();

    QMap<QString, QCheckListItem*> tree;

    QCheckListItem* root = new QCheckListItem( this, i18n( "Metadata" ), QCheckListItem::CheckBox );

    for( StringSet::const_iterator keysIt = keys.begin(); keysIt != keys.end(); ++keysIt ) {
        QStringList subKeys = QStringList::split( QString::fromLatin1("."), *keysIt);
        QCheckListItem* parent = root;
        QString path = QString::null;
        for( QStringList::Iterator subKeyIt = subKeys.begin(); subKeyIt != subKeys.end(); ++subKeyIt ) {
            if ( !path.isNull() )
                path += QString::fromLatin1( "." );
            path +=  *subKeyIt;
            if ( tree.contains( path ) )
                parent = tree[path];
            else {
                if ( parent == 0 )
                    parent = new QCheckListItem( this, *subKeyIt, QCheckListItem::CheckBox );
                else
                    parent = new QCheckListItem( parent, *subKeyIt, QCheckListItem::CheckBox );
                parent->setText( 1, path ); // This is simply to make the implementation of selected easier.
                tree.insert( path, parent );
            }
        }
    }

    if ( QListViewItem* item = firstChild() )
        item->setOpen( true );
}

#include "TreeView.moc"
