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

namespace Exif
{

class Database {

public:
    static Database* instance();
    static void setup();

    void insert( const QString& filename, Exiv2::ExifData );
    static bool isAvailable();

protected:
    static QString exifDBFile();
    static void openDatabase();
    static void populateDatabase();

private:
    Database();
    static Database* _instance;
};

}

#endif /* EXIFDATABASE_H */

