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
#include "Exif/TreeView.h"
#include "Utilities/Set.h"
#include <qmap.h>
#include <qstringlist.h>
#include "Exif/Info.h"

using Utilities::StringSet;

Exif::TreeView::TreeView( const QString& title, QWidget* parent, const char* name )
    :Q3ListView( parent, name )
{
    addColumn( title );
    reload();
    connect( this, SIGNAL( clicked( Q3ListViewItem* ) ), this, SLOT( toggleChildren( Q3ListViewItem* ) ) );
}

void Exif::TreeView::toggleChildren( Q3ListViewItem* parent )
{
    if ( !parent )
        return;

    Q3CheckListItem* par = static_cast<Q3CheckListItem*>( parent );
    bool on = par->isOn();
    for ( Q3ListViewItem* child = parent->firstChild(); child; child = child->nextSibling() ) {
        static_cast<Q3CheckListItem*>(child)->setOn( on );
        toggleChildren( child );
    }
}

StringSet Exif::TreeView::selected()
{
    StringSet result;
    for ( Q3ListViewItemIterator it( this ); *it; ++it ) {
        if ( static_cast<Q3CheckListItem*>( *it )->isOn() )
            result.insert( (*it)->text( 1 ) );
    }
    return result;
}

void Exif::TreeView::setSelectedExif( const StringSet& selected )
{
    for ( Q3ListViewItemIterator it( this ); *it; ++it ) {
        bool on = selected.contains( (*it)->text(1) );
        static_cast<Q3CheckListItem*>(*it)->setOn( on );
    }
}

void Exif::TreeView::reload()
{
    clear();
    setRootIsDecorated( true );
    StringSet keys = Exif::Info::instance()->availableKeys();

    QMap<QString, Q3CheckListItem*> tree;

    for( StringSet::const_iterator keysIt = keys.begin(); keysIt != keys.end(); ++keysIt ) {
        QStringList subKeys = (*keysIt).split(QLatin1String("."));
        Q3CheckListItem* parent = 0;
        QString path = QString::null;
        for( QStringList::Iterator subKeyIt = subKeys.begin(); subKeyIt != subKeys.end(); ++subKeyIt ) {
            if ( !path.isNull() )
                path += QString::fromLatin1( "." );
            path +=  *subKeyIt;
            if ( tree.contains( path ) )
                parent = tree[path];
            else {
                if ( parent == 0 )
                    parent = new Q3CheckListItem( this, *subKeyIt, Q3CheckListItem::CheckBox );
                else
                    parent = new Q3CheckListItem( parent, *subKeyIt, Q3CheckListItem::CheckBox );
                parent->setText( 1, path ); // This is simply to make the implementation of selected easier.
                tree.insert( path, parent );
            }
        }
    }

    if ( Q3ListViewItem* item = firstChild() )
        item->setOpen( true );
}

#include "TreeView.moc"
