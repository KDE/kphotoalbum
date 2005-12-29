#include "XMLCategory.h"
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

int XMLDB::XMLCategory::idForName( const QString& name ) const
{
    return _idMap[name];
}

void XMLDB::XMLCategory::initIdMap()
{
    int i = 0;
    _idMap.clear();
    for( QStringList::Iterator it = _items.begin(); it != _items.end(); ++it ) {
        _idMap.insert( *it, i );
        ++i;
    }
}

void XMLDB::XMLCategory::setIdMapping( const QString& name, int id )
{
    _nameMap.insert( id, name );
}

QString XMLDB::XMLCategory::nameForId( int id ) const
{
    return _nameMap[id];
}

#include "XMLCategory.moc"
