#ifndef EXIFDATABASE_H
#define EXIFDATABASE_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qpair.h>

class QSqlQuery;
namespace Exiv2 {
    class ExifData;
}

typedef QPair<int,int> Rational;
typedef QValueList<Rational> RationalList;

class ExifDatabase {

public:
    static ExifDatabase* instance();
    void setup();
    void insert( const QString& filename, Exiv2::ExifData );
    RationalList rationalValue( const QString& tag );

private:
    ExifDatabase();
    static ExifDatabase* _instance;
};

#endif /* EXIFDATABASE_H */

