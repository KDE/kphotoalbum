#include "Exif/TreeView.h"
#include "Utilities/Set.h"
#include <qmap.h>
#include <qstringlist.h>
#include <klocale.h>
#include "Exif/Info.h"

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

Set<QString> Exif::TreeView::selected()
{
    Set<QString> result;
    for ( QListViewItemIterator it( this ); *it; ++it ) {
        if ( static_cast<QCheckListItem*>( *it )->isOn() )
            result.insert( (*it)->text( 1 ) );
    }
    return result;
}

void Exif::TreeView::setSelected( const Set<QString>& selected )
{
    for ( QListViewItemIterator it( this ); *it; ++it ) {
        bool on = selected.contains( (*it)->text(1) );
        static_cast<QCheckListItem*>(*it)->setOn( on );
    }
}

void Exif::TreeView::reload()
{
    clear();
    Set<QString> keys = Exif::Info::instance()->availableKeys();

    QMap<QString, QCheckListItem*> tree;

    for( Set<QString>::Iterator keysIt = keys.begin(); keysIt != keys.end(); ++keysIt ) {
        QStringList subKeys = QStringList::split( QString::fromLatin1("."), *keysIt);
        QCheckListItem* parent = 0;
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
