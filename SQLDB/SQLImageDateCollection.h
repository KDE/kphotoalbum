#ifndef SQLIMAGEDATERANGECOLLECTION_H
#define SQLIMAGEDATERANGECOLLECTION_H
#include "DB/ImageDateCollection.h"
#include "Connection.h"
#include "QueryHelper.h"

namespace SQLDB
{

class SQLImageDateCollection :public DB::ImageDateCollection
{
public:
    explicit SQLImageDateCollection(Connection& connection);

    virtual DB::ImageCount count( const DB::ImageDate& date );
    virtual QDateTime lowerLimit() const;
    virtual QDateTime upperLimit() const;

protected:
    QueryHelper _qh;
};

}

#endif /* SQLIMAGEDATERANGECOLLECTION_H */

