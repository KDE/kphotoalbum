#include "imageinfo.h"
#include <qfileinfo.h>
#include <qimage.h>
#include <qdom.h>
#include "options.h"
#include "util.h"


ImageInfo::ImageInfo()
{
}

ImageInfo::ImageInfo( const QString& indexDirectory, const QString& fileName )
    : _indexDirectory( indexDirectory ), _fileName( fileName ), _visible( true )
{
    if ( _indexDirectory.endsWith( "/" ) )
         _indexDirectory = _indexDirectory.mid( 0, _indexDirectory.length()-1 );

    QFileInfo fi( indexDirectory+ "/" + fileName );
    _label = fi.baseName();
    _quality = _angle = 0;
}

ImageInfo::ImageInfo( const QString& indexDirectory, const QString& fileName, QDomElement elm )
    : _indexDirectory( indexDirectory ), _fileName( fileName ), _visible( true )
{
    if ( _indexDirectory.endsWith( "/" ) )
         _indexDirectory = _indexDirectory.mid( 0, _indexDirectory.length()-1 );

    QFileInfo fi( indexDirectory+ "/" + fileName );
    _label = elm.attribute( "label",  _label );
    _description = elm.attribute( "description" );

    int yearFrom = 0, monthFrom = 0,  dayFrom = 0, yearTo = 0, monthTo = 0,  dayTo = 0;

    if ( Options::instance()->trustTimeStamps() )  {
        QDate date = fi.created().date();
        yearFrom = date.year();
        monthFrom = date.month();
        dayFrom = date.day();
    }
    _startDate.setYear( elm.attribute( "yearFrom", QString::number( yearFrom) ).toInt() );
    _startDate.setMonth( elm.attribute( "monthFrom", QString::number(monthFrom) ).toInt() );
    _startDate.setDay( elm.attribute( "dayFrom", QString::number(dayFrom) ).toInt() );

    _endDate.setYear( elm.attribute( "yearTo", QString::number(yearTo) ).toInt() );
    _endDate.setMonth( elm.attribute( "monthTo", QString::number(monthTo) ).toInt() );
    _endDate.setDay( elm.attribute( "dayTo", QString::number(dayTo) ).toInt() );

    _quality = elm.attribute( "quality", "0" ).toInt();
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


void ImageInfo::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
}

void ImageInfo::addOption( const QString& key, const QStringList& value )
{
    _options[key] += value;
}

bool ImageInfo::hasOption( const QString& key, const QString& value )
{
    return _options[key].contains(value);
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

QString ImageInfo::fileName( bool relative )
{
    if (relative)
        return _fileName;
    else
        return _indexDirectory + "/" + _fileName;
}

QDomElement ImageInfo::save( QDomDocument& doc )
{
    QDomElement elm = doc.createElement( "Image" );
    elm.setAttribute( "file",  fileName( true ) );
    elm.setAttribute( "label",  _label );
    elm.setAttribute( "description", _description );

    elm.setAttribute( "yearFrom", _startDate.year() );
    elm.setAttribute( "monthFrom",  _startDate.month() );
    elm.setAttribute( "dayFrom",  _startDate.day() );

    elm.setAttribute( "yearTo", _endDate.year() );
    elm.setAttribute( "monthTo",  _endDate.month() );
    elm.setAttribute( "dayTo",  _endDate.day() );

    elm.setAttribute( "quality",  _quality );
    elm.setAttribute( "angle",  _angle );
    Util::writeOptions( doc, elm, _options );
    return elm;
}

void ImageInfo::rotate( int degrees )
{
    _angle += degrees + 360;
    _angle = _angle % 360;
}

int ImageInfo::angle() const
{
    return _angle;
}

void ImageInfo::setStartDate( const ImageDate& date )
{
    _startDate = date;
}

void ImageInfo::setEndDate( const ImageDate& date )
{
    _endDate = date;
}

ImageDate& ImageInfo::startDate()
{
    return _startDate;
}

ImageDate& ImageInfo::endDate()
{
    return _endDate;
}

QString ImageInfo::indexDirectory() const
{
    return _indexDirectory;
}

void ImageInfo::setVisible( bool b )
{
    _visible = b;
}

bool ImageInfo::visible() const
{
    return _visible;
}
