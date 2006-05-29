#ifndef SQLIMAGEDATERANGECOLLECTION_H
#define SQLIMAGEDATERANGECOLLECTION_H
#include "DB/ImageDateCollection.h"

class SQLImageDateCollection :public DB::ImageDateCollection
{
public:
    virtual DB::ImageCount count( const DB::ImageDate& date );
    virtual QDateTime lowerLimit() const;
    virtual QDateTime upperLimit() const;
};


#endif /* SQLIMAGEDATERANGECOLLECTION_H */

