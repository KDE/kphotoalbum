#include "imagesearchinfo.h"

ImageSearchInfo::ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate,
                                  const QString& persons, const QString& locations, const QString& keywords,
                                  const QString& label, const QString& description )
    : _startDate( startDate ),_endDate( endDate ),
      _persons( persons ),_locations( locations ),_keywords( keywords ),_label( label ), _description( description )
{
}

ImageDate ImageSearchInfo::startDate() const
{
    return _startDate;
}


ImageDate ImageSearchInfo::endDate() const
{
    return _endDate;
}

QString ImageSearchInfo::persons() const
{
    return _persons;
}

QString ImageSearchInfo::locations() const
{
    return _locations;
}

QString ImageSearchInfo::keywords() const
{
    return _keywords;
}

QString ImageSearchInfo::label() const
{
    return _label;
}

QString ImageSearchInfo::description() const
{
    return _description;
}

ImageSearchInfo::ImageSearchInfo()
{
}

