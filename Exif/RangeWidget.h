#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

#include <qobject.h>
#include "Exif/SearchInfo.h"
class QGrid;
class QComboBox;

namespace Exif{

class RangeWidget :public QObject{
    Q_OBJECT

public:
    class Value
    {
    public:
        Value() {}
        Value( double value, const QString& text ) :value( value ), text( text ) {}
        double value;
        QString text;
    };

    typedef QValueList<Value> ValueList ;

    RangeWidget( const QString& text, const QString& searchTag, const ValueList& list, QGrid* parent);
    Exif::SearchInfo::Range range() const;

protected slots:
    void slotUpdateTo( int index );

protected:
    QString tagToLabel( const QString& tag );

private:
    QString _searchTag;
    QComboBox* _from;
    QComboBox* _to;
    ValueList _list;
};

}

#endif /* RANGEWIDGET_H */

