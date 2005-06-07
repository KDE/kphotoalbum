#include "xmlcategory.h"
#include "imagedb.h"
#include "options.h"
#include "membermap.h"

XMLDB::XMLCategory::XMLCategory( const QString& name, const QString& icon, ViewSize size, ViewType type, bool show )
    : _name( name ), _icon( icon ), _show( show ), _size( size ), _type( type ), _isSpecial(false)
{
}

QString XMLDB::XMLCategory::name() const
{
    return _name;
}

void XMLDB::XMLCategory::setName( const QString& name )
{
    _name = name;
}

QString XMLDB::XMLCategory::iconName() const
{
    return _icon;
}

void XMLDB::XMLCategory::setIconName( const QString& name )
{
    _icon = name;
    emit changed();
}

void XMLDB::XMLCategory::setViewSize( ViewSize size )
{
    _size = size;
    emit changed();
}

void XMLDB::XMLCategory::setViewType( ViewType type )
{
    _type = type;
    emit changed();
}

XMLDB::XMLCategory::ViewSize XMLDB::XMLCategory::viewSize() const
{
    return _size;
}

XMLDB::XMLCategory::ViewType XMLDB::XMLCategory::viewType() const
{
    return _type;
}

void XMLDB::XMLCategory::setDoShow( bool b )
{
    _show = b;
    emit changed();
}

bool XMLDB::XMLCategory::doShow() const
{
    return _show;
}

void XMLDB::XMLCategory::setSpecialCategory( bool b )
{
    _isSpecial = b;
}

bool XMLDB::XMLCategory::isSpecialCategory() const
{
    return _isSpecial;
}

void XMLDB::XMLCategory::setItems( const QStringList& items )
{
    _items = items;
}

void XMLDB::XMLCategory::removeItem( const QString& item )
{
    _items.remove( item );
    emit itemRemoved( item );
}

void XMLDB::XMLCategory::renameItem( const QString& oldValue, const QString& newValue )
{
    _items.remove( oldValue );
    addItem( newValue );
    emit itemRenamed( oldValue, newValue );
}

void XMLDB::XMLCategory::addItem( const QString& item )
{
    if (_items.contains( item ) )
        _items.remove( item );
    _items.prepend( item );
}

QStringList XMLDB::XMLCategory::items() const
{
    return _items;
}

QStringList XMLDB::XMLCategory::itemsInclGroups() const
{
    // values including member groups

    QStringList items = _items;

    // add the groups to the list too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = ImageDB::instance()->memberMap().groups( name() );
    for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        if ( ! items.contains(  *it ) )
            items << *it ;
    };
    if ( Options::instance()->viewSortType() == Options::SortAlpha )
        items.sort();
    return items;
}
