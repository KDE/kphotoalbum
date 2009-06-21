#include "NoOtherItemsCategoryMatcher.h"
#include "ImageInfo.h"

DB::NoOtherItemsCategoryMatcher::NoOtherItemsCategoryMatcher( const QString& category, bool sign )
{
    _category = category;
    _sign = sign;
}

bool DB::NoOtherItemsCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
    Q_ASSERT( _shouldPrepareMatchedSet );
    bool allMatched = true;
    Q_FOREACH(const QString& item, info->itemsOfCategory(_category))
    {
        if (!alreadyMatched[_category].contains(item))
        {
            allMatched = false;
            break;
        }
    }
    return _sign ? allMatched : !allMatched;
}

void DB::NoOtherItemsCategoryMatcher::debug( int level ) const
{
    qDebug("%s%s:EMPTY", qPrintable(spaces(level)), qPrintable(_category) );
}

bool DB::NoOtherItemsCategoryMatcher::hasEmptyMatcher() const
{
    return true;
}

