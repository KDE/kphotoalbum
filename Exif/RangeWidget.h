#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

#include <qwidget.h>
#include "Exif/SearchInfo.h"
class QComboBox;

namespace Exif{

class RangeWidget : public QWidget {
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

    RangeWidget( const QString& text, const QString& searchTag, const ValueList& list, QWidget* parent, const char* name = 0 );
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

