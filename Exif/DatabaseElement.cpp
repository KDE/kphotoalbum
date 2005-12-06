#include "DatabaseElement.h"
#include <qsqlquery.h>
#include <exiv2/exif.hpp>

static QString replaceDotWithUnderscore( const char* cstr )
{
    QString str( QString::fromLatin1( cstr ) );
    return str.replace( QString::fromLatin1( "." ), QString::fromLatin1( "_" ) );
}

Exif::StringExifElement::StringExifElement( const char* tag )
    : _tag( tag )
{
}


QString Exif::StringExifElement::createString()
{
    return QString::fromLatin1( "%1 string" ).arg( replaceDotWithUnderscore( _tag ) );
}


QString Exif::StringExifElement::queryString()
{
    return QString::fromLatin1( "?" );
}


void Exif::StringExifElement::bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
{
    query->bindValue( counter++, data[_tag].toString().c_str() );
}


Exif::IntExifElement::IntExifElement( const char* tag )
 : _tag( tag )
{
}


QString Exif::IntExifElement::createString()
{
    return QString::fromLatin1( "%1 int" ).arg( replaceDotWithUnderscore( _tag ) );
}


QString Exif::IntExifElement::queryString()
{
    return QString::fromLatin1( "?" );
}


void Exif::IntExifElement::bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
{
    query->bindValue( counter++, (int) data[_tag].toLong() );
}


Exif::RationalExifElement::RationalExifElement( const char* tag )
 : _tag( tag )
{
}


QString Exif::RationalExifElement::createString()
{
    return QString::fromLatin1( "%1 float" ).arg( replaceDotWithUnderscore( _tag ) );
}

QString Exif::RationalExifElement::queryString()
{
    return QString::fromLatin1( "?" );
}


void Exif::RationalExifElement::bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
{
    query->bindValue( counter++, 1.0 * data[_tag].toRational().first / data[_tag].toRational().second);
}

