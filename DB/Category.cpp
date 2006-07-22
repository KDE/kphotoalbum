#include "Category.h"
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include "DB/ImageDB.h"
#include "Settings/SettingsData.h"
#include "DB/MemberMap.h"
#include "CategoryItem.h"

QPixmap DB::Category::icon( int size ) const
{
    return KGlobal::iconLoader()->loadIcon( iconName(), KIcon::Desktop, size );
}


/**
   If one person from say Denmark sends a database to a person from say germany, then the title of
   Persons, Locations, and Keywords will still be translated correct, when this function is used.
*/
QString DB::Category::text() const
{
    if ( name() == QString::fromLatin1( "Persons" ) )
        return i18n("Persons");
    else if ( name() == QString::fromLatin1( "Locations" ) )
        return i18n("Locations");
    else if ( name() == QString::fromLatin1( "Keywords" ) )
        return i18n("Keywords");
    else if ( name() == QString::fromLatin1( "Folder" ) )
        return i18n("Folder");
    else if ( name() == QString::fromLatin1( "Tokens" ) )
        return i18n("Tokens");
    else
        return name();
}

QStringList DB::Category::itemsInclCategories() const
{
    // values including member groups

    QStringList items = this->items();

    // add the groups to the list too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = DB::ImageDB::instance()->memberMap().groups( name() );
    for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        if ( ! items.contains(  *it ) )
            items << *it ;
    };

    return items;
}

#ifdef TEMPORARILY_REMOVED
DB::CategoryItem* createParentItem( DB::CategoryItem* top, QMap<QString,DB::CategoryItem*>& parentMap,
                                    const QMap<QString,QStringList>& inverseGroupMap, const QString& item )
{
    if ( parentMap.contains( item ) )
        return parentMap[item];

    QValueList<DB::CategoryItem*> parents;
    if ( inverseGroupMap.contains( item ) ) {
        QStringList parentNames = inverseGroupMap[item];
        for( QStringList::ConstIterator parentIt = parentNames.begin(); parentIt != parentNames.end(); ++parentIt ) {
            DB::CategoryItem* parentItem = createParentItem( top, parentMap, inverseGroupMap, *parentIt  );
            parents.append( parentItem );
        }
    }
    else
        parents.append( top );

    QValueList<DB::CategoryItem*> res;
    for( QValueList<DB::CategoryItem*>::ConstIterator parentIt = parents.begin(); parentIt != parents.end(); ++parentIt ) {
        DB::CategoryItem* child = new DB::CategoryItem(  );
        parent->_subcategories.append( res );
        parentMap.insert( item, res );
        return res;
    }
}
#endif


QValueList<DB::CategoryItem*> createItems( const QString& name, DB::CategoryItem* top, const QMap<QString,QStringList>& inverseMap,
                                           QMap<QString, QValueList<DB::CategoryItem*> >* parentMap )
{
    QValueList<DB::CategoryItem*> res;
    if ( parentMap->contains( name ) )
        return (*parentMap)[name];

    if ( inverseMap.contains( name ) ) {
        QStringList groups = inverseMap[name];
        for( QStringList::ConstIterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
            QValueList<DB::CategoryItem*> parentList = createItems( *groupIt, top, inverseMap, parentMap );
            for( QValueList<DB::CategoryItem*>::ConstIterator parentItemIt = parentList.begin(); parentItemIt != parentList.end(); ++parentItemIt ) {
                DB::CategoryItem* child = new DB::CategoryItem( name );
                (*parentItemIt)->_subcategories.append( child );
                res.append( child );
            }
        }
    }
    else {
        // No group contains this item as a member
        DB::CategoryItem* child = new DB::CategoryItem( name );
        top->_subcategories.append( child );
        res.append( child );
    }

    (*parentMap)[name] += res;
    return res;
}

DB::CategoryItem* DB::Category::itemsCategories() const
{
    CategoryItem* result = new CategoryItem( QString::fromLatin1("top") );
    QStringList items = this->items();
    QMap<QString,QStringList> inverseGroupMap = ImageDB::instance()->memberMap().inverseMap( name() );
    QMap<QString, QValueList<DB::CategoryItem*> > parentMap;

    for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt )
        createItems( *itemIt, result, inverseGroupMap, &parentMap );


#ifdef TEMPORARILY_REMOVED
    QMap<QString,QStringList> inverseGroupMap = ImageDB::instance()->memberMap().inverseMap( name() );
    QMap<QString, CategoryItem*> parentMap;
    for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        if ( inverseGroupMap.contains( *itemIt ) ) {
            QStringList groups = inverseGroupMap[*itemIt];
            for( QStringList::ConstIterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
                CategoryItem* parentItem = createParentItem( result, parentMap, inverseGroupMap, *groupIt );
                qDebug("Adding %s as child of %s", (*itemIt).latin1(), parentItem->_name.latin1());
                parentItem->_subcategories.append( new CategoryItem( *itemIt ) );
            }
        }
        else
            result->_subcategories.append( new CategoryItem( *itemIt ) );
    }
#endif

    return result;
}

#ifdef TEMPORARILY_REMOVED
DB::CategoryItem* createParentItem( DB::CategoryItem* top, QMap<QString,DB::CategoryItem*>& parentMap,
                                    const QMap<QString,QStringList>& inverseGroupMap, const QString& item )
{
    if ( parentMap.contains( item ) )
        return parentMap[item];

    QValueVector<DB::CategoryItem*> parents;
    if ( inverseGroupMap.contains( item ) ) {
        QStringList parentNames = inverseGroupMap[item];
        for( QStringList::ConstIterator parentIt = parentNames.begin(); parentIt != parentNames.end(); ++parentIt ) {
            DB::CategoryItem* parentItem = createParentItem( top, parentMap, inverseGroupMap, *parentIt  );
            parents.append( parentItem );
        }
    }
    else
        parents.append( top );

    QValueList<DB::CategoryItem*> res;
    for( QValueList<DB::CategoryItem*>::ConstIterator parentIt = parents.begin(); parentIt != parents.end(); ++parentIt ) {
        DB::CategoryItem* child = new DB::CategoryItem(  );
        parent->_subcategories.append( res );
        parentMap.insert( item, res );
        return res;
    }
}

DB::CategoryItem* DB::Category::itemsCategories() const
{
    CategoryItem* result = new CategoryItem( QString::fromLatin1("top") );
    QStringList items = this->items();

    QMap<QString,QStringList> inverseGroupMap = ImageDB::instance()->memberMap().inverseMap( name() );
    QMap<QString, CategoryItem*> parentMap;
    for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        if ( inverseGroupMap.contains( *itemIt ) ) {
            QStringList groups = inverseGroupMap[*itemIt];
            for( QStringList::ConstIterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
                CategoryItem* parentItem = createParentItem( result, parentMap, inverseGroupMap, *groupIt );
                qDebug("Adding %s as child of %s", (*itemIt).latin1(), parentItem->_name.latin1());
                parentItem->_subcategories.append( new CategoryItem( *itemIt ) );
            }
        }
        else
            result->_subcategories.append( new CategoryItem( *itemIt ) );
    }

    return result;
}
#endif

#include "Category.moc"
class QString;
