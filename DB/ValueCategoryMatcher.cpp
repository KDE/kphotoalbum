#include "ValueCategoryMatcher.h"
#include "ImageDB.h"
#include "MemberMap.h"

void DB::ValueCategoryMatcher::debug(int level) const
{
    qDebug("%s%s: %s", qPrintable(spaces(level)), qPrintable(_category), qPrintable(_option));
}

DB::ValueCategoryMatcher::ValueCategoryMatcher( const QString& category, const QString& value, bool sign )
{
    _category = category ;
    _option = value;
    _sign = sign;

    const MemberMap& map = DB::ImageDB::instance()->memberMap();
    const QStringList members = map.members(_category, _option, true);
    _members = members.toSet();
}

bool DB::ValueCategoryMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
    // Following block does same as the old statement:
    // info->setMatched(_category, _option)
    if ( _shouldPrepareMatchedSet ) {
        alreadyMatched[_category].insert(_option);
        alreadyMatched[_category].unite(_members);
    }

    if ( info->hasCategoryInfo( _category, _option ) ) {
        return _sign;
    }

    if ( info->hasCategoryInfo( _category, _members ) )
        return _sign;
    return !_sign;
}

bool DB::ValueCategoryMatcher::hasEmptyMatcher() const
{
    return false;
}

