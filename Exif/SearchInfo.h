#ifndef EXIFSEARCHINFO_H
#define EXIFSEARCHINFO_H

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qpair.h>
#include "Utilities/Set.h"

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
    void addCamara( const QValueList< QPair<QString, QString> >& );

    void search() const;
    bool matches( const QString& fileName ) const;

protected:
    QString buildQuery() const;
    QStringList buildIntKeyQuery() const;
    QStringList buildRangeQuery() const;
    QString buildCameraSearchQuery() const;
    QString sqlForOneRangeItem( const Range& ) const;

private:
    typedef QValueList< QPair<QString, QValueList<int> > > IntKeyList;
    IntKeyList _intKeys;
    QValueList<Range> _rangeKeys;
    QValueList< QPair<QString,QString> > _cameras;
    mutable Set<QString> _matches;
    mutable bool _emptyQuery;
};

}


#endif /* EXIFSEARCHINFO_H */

