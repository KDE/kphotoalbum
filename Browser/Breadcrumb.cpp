#include "Breadcrumb.h"
#include <klocale.h>

int Browser::Breadcrumb::_count = 0;

Browser::Breadcrumb::Breadcrumb(const QString& text, bool isBeginning )
    : _index( ++_count ),_isBeginning( isBeginning), _text( text )
{
}

Browser::Breadcrumb Browser::Breadcrumb::empty()
{
    return Breadcrumb( QString() );
}

Browser::Breadcrumb Browser::Breadcrumb::home()
{
    return Breadcrumb( i18n("All"), true );
}

QString Browser::Breadcrumb::text() const
{
    return _text;
}

bool Browser::Breadcrumb::isBeginning() const
{
    return _isBeginning;
}

bool Browser::Breadcrumb::operator==( const Breadcrumb& other ) const
{
    return other._index == _index;
}

bool Browser::Breadcrumb::operator!=( const Breadcrumb& other ) const
{
    return !(other == *this );
}
