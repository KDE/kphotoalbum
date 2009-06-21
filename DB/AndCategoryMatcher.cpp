#include "AndCategoryMatcher.h"
#include "ImageInfo.h"

bool DB::AndCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
     for( QList<CategoryMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if (!(*it)->eval(info, alreadyMatched))
            return false;
    }
    return true;
}

void DB::AndCategoryMatcher::debug( int level ) const
{
    qDebug("%sAND:", qPrintable(spaces(level)) );
    ContainerCategoryMatcher::debug( level + 1 );
}

