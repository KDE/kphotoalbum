#include "imagesearchinfo.h"

ImageSearchInfo::ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate, int startQuality, int endQuality,
                                  const QString& persons, const QString& locations, const QString& keywords, const QString& items,
                                  const QString& label, const QString& description )
    : _startDate( startDate ),_endDate( endDate ),_startQuality( startQuality ),_endQuality( endQuality ),
      _persons( persons ),_locations( locations ),_keywords( keywords ),_items( items ),_label( label ), _description( description )
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

int ImageSearchInfo::startQuality() const
{
    return _startQuality;
}

int ImageSearchInfo::endQuality() const
{
    return _endQuality;
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

QString ImageSearchInfo::items() const
{
    return _items;
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
    :_startQuality(0), _endQuality(0)
{
}

