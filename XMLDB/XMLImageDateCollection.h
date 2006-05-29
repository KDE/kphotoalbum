#ifndef XMLIMAGEDATERANGECOLLECTION_H
#define XMLIMAGEDATERANGECOLLECTION_H
#include "DB/ImageDateCollection.h"

class XMLImageDateCollection :public DB::ImageDateCollection
{
public:
    XMLImageDateCollection();
    XMLImageDateCollection( const QStringList& );

public:
    virtual DB::ImageCount count( const DB::ImageDate& range );
    virtual QDateTime lowerLimit() const;
    virtual QDateTime upperLimit() const;

private:
    void append( const DB::ImageDate& );

    QValueList<DB::ImageDate> _dates;
    QMap<DB::ImageDate,DB::ImageCount> _cache;
    mutable bool _dirtyLower, _dirtyUpper;
};


#endif /* XMLIMAGEDATERANGECOLLECTION_H */

