#include "optionmatcher.h"
#include "options.h"
OptionValueMatcher::OptionValueMatcher( const QString& optionGroup, const QString& option )
    :_optionGroup( optionGroup ), _option( option )
{
}

bool OptionValueMatcher::eval( ImageInfo* info )
{
    if ( info->hasOption( _optionGroup, _option ) )
        return true;

    QStringList list = Options::instance()->memberMap().members( _optionGroup, _option, true );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( info->hasOption( _optionGroup, *it ) )
            return true;
    }

    return false;
}



OptionEmptyMatcher::OptionEmptyMatcher( const QString& optionGroup )
    :_optionGroup( optionGroup )
{
}

bool OptionEmptyMatcher::eval( ImageInfo* info )
{
    return (info->optionValue( _optionGroup ).count() == 0);
}



void OptionContainerMatcher::addElement( OptionMatcher* element )
{
    _elements.append( element );
}

bool OptionAndMatcher::eval( ImageInfo* info )
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ( !(*it)->eval( info ) )
            return false;
    }
    return true;
}



bool OptionOrMatcher::eval( ImageInfo* info )
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ( (*it)->eval( info ) )
            return true;
    }
    return false;
}



OptionNotMatcher::OptionNotMatcher( OptionMatcher* element )
    :_element( element )
{
}

bool OptionNotMatcher::eval( ImageInfo* info )
{
    return !_element->eval( info );
}

OptionMatcher* OptionValueMatcher::optimize()
{
    return this;
}

OptionMatcher* OptionEmptyMatcher::optimize()
{
    return this;
}

OptionMatcher* OptionContainerMatcher::optimize()
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ) {
        QValueList<OptionMatcher*>::Iterator matcher = it;
        ++it;

        (*matcher) = (*matcher)->optimize();
        if ( *matcher == 0 )
            _elements.remove( matcher );
    }

    if ( _elements.count() == 0 ) {
        delete this;
        return 0;
    }
    else if ( _elements.count() == 1 ) {
        OptionMatcher* res = _elements[0]->optimize();
        _elements.clear();
        delete this;
        return res;
    }

    else
        return this;

}

OptionMatcher* OptionNotMatcher::optimize()
{
    _element = _element->optimize();
    if ( _element == 0 ) {
        delete this;
        return 0;
    }
    return this;
}

OptionContainerMatcher::~OptionContainerMatcher()
{
    for( uint i = 0; i < _elements.count(); ++i )
        delete _elements[i];
}
