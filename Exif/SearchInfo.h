#ifndef EXIFSEARCHINFO_H
#define EXIFSEARCHINFO_H

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qpair.h>

namespace Exif {

class SearchInfo  {

public:
    class Range
    {
    public:
        Range() {}
        Range( const QString& key );
        bool isLowerMin, isLowerMax, isUpperMin, isUpperMax;
        double min, max;
        QString key;
    };

    void addSearchKey( const QString& key, const QValueList<int> values );
    void addRangeKey( const Range& range );

    QStringList matches();
    QString buildQuery();

protected:
    QStringList buildIntKeyQuery();
    QStringList buildRangeQuery();
    QString sqlForOneRangeItem( const Range& );

private:
    typedef QValueList< QPair<QString, QValueList<int> > > IntKeyList;
    IntKeyList _intKeys;
    typedef QValueList<Range> RangeList;
    RangeList _rangeKeys;
};

}


#endif /* EXIFSEARCHINFO_H */

