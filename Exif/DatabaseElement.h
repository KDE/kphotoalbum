#ifndef DATABASEELEMENT_H
#define DATABASEELEMENT_H

#include <qstring.h>
namespace Exiv2
{
    class ExifData;
}
class QSqlQuery;

namespace Exif {

class DatabaseElement
{
public:
    virtual QString createString() = 0; // Exif_Photo_FNumber_denominator int, Exif_Photo_FNumber_nominator int
    virtual QString queryString() = 0; // ?, ?
    virtual void bindValues( QSqlQuery*, int& counter, Exiv2::ExifData& data ) = 0; // bind values
};

class StringExifElement :public DatabaseElement
{
public:
    StringExifElement( const char* tag );
    QString createString();
    QString queryString();
    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data );

private:
    const char* _tag;
};

class IntExifElement :public DatabaseElement
{
public:
    IntExifElement( const char* tag );
    QString createString();
    QString queryString();
    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data );

private:
    const char* _tag;
};


class RationalExifElement :public DatabaseElement
{
public:
    RationalExifElement( const char* tag );
    virtual QString createString();
    virtual QString queryString();
    virtual void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data );

private:
    const char* _tag;
};


}



#endif /* DATABASEELEMENT_H */

