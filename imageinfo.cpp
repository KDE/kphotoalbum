#include "imageinfo.h"
#include <qfileinfo.h>
#include <qimage.h>
#include <qdom.h>
#include "options.h"
#include "util.h"


ImageInfo::ImageInfo()
{
}

ImageInfo::ImageInfo( const QString& fileName, QDomElement elm )
    : _fileName( fileName )
{
    QFileInfo fi( fileName );
    _label = elm.attribute( "label",  fi.baseName() );
    _description = elm.attribute( "description" );

    int yearFrom = -1, monthFrom = -1,  dayFrom = -1, yearTo = -1, monthTo = -1,  dayTo = -1;

    if ( Options::instance()->trustTimeStamps() )  {
        QDate date = fi.created().date();
        yearFrom = date.year();
        monthFrom = date.month();
        dayFrom = date.day();
    }
    _yearFrom = elm.attribute( "yearFrom", QString::number(yearFrom) ).toInt();
    _monthFrom = elm.attribute( "monthFrom", QString::number(monthFrom) ).toInt();
    _dayFrom = elm.attribute( "dayFrom", QString::number(dayFrom) ).toInt();

    _yearTo = elm.attribute( "yearTo", QString::number(yearTo) ).toInt();
    _monthTo = elm.attribute( "monthTo", QString::number(monthTo) ).toInt();
    _dayTo = elm.attribute( "dayTo", QString::number(dayTo) ).toInt();

    _quality = elm.attribute( "quality", "3" ).toInt();
    _angle = elm.attribute( "angle", "0" ).toInt();
    Util::readOptions( elm, &_options );
}

void ImageInfo::setLabel( const QString& desc )
{
    _label = desc;
}

QString ImageInfo::label() const
{
    return _label;
}

void ImageInfo::setDescription( const QString& desc )
{
    _description = desc;
}

QString ImageInfo::description() const
{
    return _description;
}

void ImageInfo::setYearFrom( int year ) {
    _yearFrom = year;
}

void ImageInfo::setMonthFrom( int month ) {
    _monthFrom = month;
}

void ImageInfo::setDayFrom( int day ) {
    _dayFrom = day;
}

int ImageInfo::yearFrom() const
{
    return _yearFrom;
}

int ImageInfo::monthFrom() const
{
    return _monthFrom;
}

int ImageInfo::dayFrom() const
{
    return _dayFrom;
}

void ImageInfo::setYearTo( int year ) {
    _yearTo = year;
}

void ImageInfo::setMonthTo( int month ) {
    _monthTo = month;
}

void ImageInfo::setDayTo( int day ) {
    _dayTo = day;
}

int ImageInfo::yearTo() const
{
    return _yearTo;
}

int ImageInfo::monthTo() const
{
    return _monthTo;
}

int ImageInfo::dayTo() const
{
    return _dayTo;
}

void ImageInfo::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
}

void ImageInfo::addOption( const QString& key, const QStringList& value )
{
    _options[key] += value;
}

QStringList ImageInfo::optionValue( const QString& key ) const
{
    return _options[key];
}

void ImageInfo::setQuality( int quality )
{
    _quality = quality;
}

int ImageInfo::quality() const
{
    return _quality;
}

QString ImageInfo::fileName()
{
    return _fileName;
}

QDomElement ImageInfo::save( QDomDocument& doc )
{
    QDomElement elm = doc.createElement( "Image" );
    elm.setAttribute( "file",  QFileInfo( _fileName ).fileName() );
    elm.setAttribute( "label",  _label );
    elm.setAttribute( "description", _description );

    elm.setAttribute( "yearFrom", _yearFrom );
    elm.setAttribute( "monthFrom",  _monthFrom );
    elm.setAttribute( "dayFrom",  _dayFrom );

    elm.setAttribute( "yearTo", _yearTo );
    elm.setAttribute( "monthTo",  _monthTo );
    elm.setAttribute( "dayTo",  _dayTo );

    elm.setAttribute( "quality",  _quality );
    elm.setAttribute( "angle",  _angle );
    Util::writeOptions( doc, elm, _options );
    return elm;
}

void ImageInfo::rotate( int degrees )
{
    _angle += degrees;
}

int ImageInfo::angle() const
{
    return _angle;
}











