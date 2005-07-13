#ifndef XMLIMAGEDATERANGECOLLECTION_H
#define XMLIMAGEDATERANGECOLLECTION_H
#include <imagedaterangecollection.h>

class XMLImageDateRangeCollection :public ImageDateRangeCollection
{
public:
    XMLImageDateRangeCollection();
    XMLImageDateRangeCollection( const QStringList& );

public:
    virtual ImageCount count( const ImageDateRange& range );
    virtual QDateTime lowerLimit() const;
    virtual QDateTime upperLimit() const;

private:
    void append( const ImageDateRange& );

    QValueList<ImageDateRange> _dates;
    QMap<ImageDateRange,ImageCount> _cache;
    mutable bool _dirtyLower, _dirtyUpper;
};


#endif /* XMLIMAGEDATERANGECOLLECTION_H */

