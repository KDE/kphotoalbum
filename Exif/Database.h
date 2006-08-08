#ifndef EXIFDATABASE_H
#define EXIFDATABASE_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qpair.h>
#include "Utilities/Set.h"

class QSqlDatabase;
class QSqlQuery;

namespace Exiv2 {
class ExifData;
}

typedef QPair<int,int> Rational;
typedef QValueList<Rational> RationalList;

namespace Exif
{

// ================================================================================
// IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT
// ================================================================================
//
// It is the resposibility of the methods in here to bail out in case database support
// is not available ( !isAvailable() ). This is to simplify client code.
class Database {

public:
    static Database* instance();
    static bool isAvailable();

    bool isOpen() const;
    bool isUsable() const;
    void add( const QString& fileName );
    void remove( const QString& fileName );
    Set<QString> filesMatchingQuery( const QString& query );
    QValueList< QPair<QString,QString> > cameras() const;

protected:
    static QString exifDBFile();
    void openDatabase();
    void populateDatabase();
    static QString connectionName();
    void insert( const QString& filename, Exiv2::ExifData );
    void offerInitialize();

private:
    bool _isOpen;
    Database();
    void init();
    static Database* _instance;
    QSqlDatabase* _db;
};

}

#endif /* EXIFDATABASE_H */

