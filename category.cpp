#include "category.h"
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

Category::Category( const QString& name, const QString& icon, ViewSize size, ViewType type, bool show )
    : QObject( 0, "category" ),_name( name ), _icon( icon ), _show( show ), _size( size ), _type( type ), _isSpecial(false)
{
}

QString Category::name() const
{
    return _name;
}

void Category::setName( const QString& name )
{
    _name = name;
}

/**
   If one person from say Denmark sends a database to a person from say germany, then the title of
   Persons, Locations, and Keywords will still be translated correct, when this function is used.
*/
QString Category::text() const
{
    if ( _name == QString::fromLatin1( "Persons" ) )
        return i18n("Persons");
    else if ( _name == QString::fromLatin1( "Locations" ) )
        return i18n("Locations");
    else if ( _name == QString::fromLatin1( "Keywords" ) )
        return i18n("Keywords");
    else if ( _name == QString::fromLatin1( "Folder" ) )
        return i18n("Folder");
    else if ( _name == QString::fromLatin1( "Tokens" ) )
        return i18n("Tokens");
    else
        return _name;
}

QString Category::iconName() const
{
    return _icon;
}

QPixmap Category::icon( int size )
{
    return KGlobal::iconLoader()->loadIcon( _icon, KIcon::Desktop, size );
}

void Category::setIconName( const QString& name )
{
    _icon = name;
    emit changed();
}

void Category::setViewSize( ViewSize size )
{
    _size = size;
    emit changed();
}

void Category::setViewType( ViewType type )
{
    _type = type;
    emit changed();
}

Category::ViewSize Category::viewSize() const
{
    return _size;
}

Category::ViewType Category::viewType() const
{
    return _type;
}

void Category::setDoShow( bool b )
{
    _show = b;
    emit changed();
}

bool Category::doShow() const
{
    return _show;
}

void Category::setSpecialCategory( bool b )
{
    _isSpecial = b;
}

bool Category::isSpecialCategory() const
{
    return _isSpecial;
}

#include "category.moc"
