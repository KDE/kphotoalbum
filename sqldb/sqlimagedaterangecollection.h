#ifndef SQLIMAGEDATERANGECOLLECTION_H
#define SQLIMAGEDATERANGECOLLECTION_H
#include <imagedaterangecollection.h>

class SQLImageDateRangeCollection :public ImageDateRangeCollection
{
public:
    virtual ImageCount count( const ImageDateRange& range );
    virtual QDateTime lowerLimit() const;
    virtual QDateTime upperLimit() const;
};


#endif /* SQLIMAGEDATERANGECOLLECTION_H */

