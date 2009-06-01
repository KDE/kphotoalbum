#include "Breadcrumb.h"

Browser::Breadcrumb::Breadcrumb( const QString& text, bool isBeginning )
    : _isBeginning( isBeginning), _text( text )
{
}

Browser::Breadcrumb Browser::Breadcrumb::empty()
{
    return Breadcrumb( QString() );
}

Browser::Breadcrumb Browser::Breadcrumb::home()
{
    return Breadcrumb( QString(), true );
}

QString Browser::Breadcrumb::text() const
{
    return _text;
}

bool Browser::Breadcrumb::isBeginning() const
{
    return _isBeginning;
}
