#include "ContainerCategoryMatcher.h"

void DB::ContainerCategoryMatcher::addElement( CategoryMatcher* element )
{
    _elements.append( element );
}

DB::ContainerCategoryMatcher::~ContainerCategoryMatcher()
{
    for( int i = 0; i < _elements.count(); ++i )
        delete _elements[i];
}

void DB::ContainerCategoryMatcher::debug( int level ) const
{
     for( QList<CategoryMatcher*>::ConstIterator it = _elements.begin(); it != _elements.end(); ++it ) {
        (*it)->debug( level );
    }
}

bool DB::ContainerCategoryMatcher::hasEmptyMatcher() const
{
    Q_FOREACH( const DB::CategoryMatcher* matcher,_elements )
        if ( matcher->hasEmptyMatcher() )
            return true;

    return false;
}

void DB::ContainerCategoryMatcher::setShouldCreateMatchedSet(bool b)
{
    _shouldPrepareMatchedSet = b;
    Q_FOREACH( DB::CategoryMatcher* matcher,_elements )
        matcher->setShouldCreateMatchedSet( b );
}

