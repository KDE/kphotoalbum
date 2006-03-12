#ifndef XMLIMAGEDATERANGECOLLECTION_H
#define XMLIMAGEDATERANGECOLLECTION_H
#include <imagedatecollection.h>

class XMLImageDateCollection :public ImageDateCollection
{
public:
    XMLImageDateCollection();
    XMLImageDateCollection( const QStringList& );

public:
    virtual ImageCount count( const ImageDate& range );
    virtual QDateTime lowerLimit() const;
    virtual QDateTime upperLimit() const;

private:
    void append( const ImageDate& );

    QValueList<ImageDate> _dates;
    QMap<ImageDate,ImageCount> _cache;
    mutable bool _dirtyLower, _dirtyUpper;
};


#endif /* XMLIMAGEDATERANGECOLLECTION_H */

