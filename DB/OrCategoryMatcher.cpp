#include "OrCategoryMatcher.h"
#include "ImageInfo.h"

bool DB::OrCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
     for( QList<CategoryMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ((*it)->eval(info, alreadyMatched))
            return true;
    }
    return false;
}

void DB::OrCategoryMatcher::debug( int level ) const
{
    qDebug("%sOR:", qPrintable(spaces(level)) );
    ContainerCategoryMatcher::debug( level + 1 );
}

