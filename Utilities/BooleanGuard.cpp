#include "BooleanGuard.h"

Utilities::BooleanGuard::BooleanGuard( bool& guard )
    : _guard( guard )
{
    if ( _guard == false ) {
        _iLocedIt =true;
        _guard = true;
    }
    else
        _iLocedIt = false;
}

Utilities::BooleanGuard::~BooleanGuard()
{
    if ( _iLocedIt )
        _guard = false;
}

bool Utilities::BooleanGuard::canContinue()
{
    return _iLocedIt;
}
