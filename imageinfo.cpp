#include "imageinfo.h"
#include <qfileinfo.h>
#include <qimage.h>
#include <qdom.h>
#include "options.h"


ImageInfo::ImageInfo()
{
}

ImageInfo::ImageInfo( const QString& fileName, QDomElement elm )
    : _fileName( fileName )
{
    QFileInfo fi( fileName );
    _label = elm.attribute( "label",  fi.baseName() );
    _description = elm.attribute( "description" );

    int year = -1, month = -1,  day = -1,  hour = -1,  minute = -1;
    if ( Options::instance()->trustFileTimeStamps() )  {
        QDate date = fi.created().date();
        QTime time = fi.created().time();
        year = date.year();
        month = date.month();
        day = date.day();
        hour = time.hour();
        minute = time.minute();
    }
    _year = elm.attribute( "year", QString::number(year) ).toInt();
    _month = elm.attribute( "month", QString::number(month) ).toInt();
    _day = elm.attribute( "day", QString::number(day) ).toInt();
    _hour = elm.attribute( "hour", QString::number(hour) ).toInt();
    _minute = elm.attribute( "minute", QString::number(minute) ).toInt();
    _quality = elm.attribute( "quality" ).toInt();
    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull(); nodeOption = nodeOption.nextSibling() )  {
        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            Q_ASSERT( elmOption.tagName() == "option" );
            QString name = elmOption.attribute( "name" );
            if ( !name.isNull() )  {
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull(); nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == "value" );
                        QString value = elmValue.attribute( "value" );
                        if ( !value.isNull() )  {
                            _options[name].append( value );
                        }
                    }
                }
            }
        }
    }
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

void ImageInfo::setDate( int year, int month, int day )
{
    _year = year;
    _month = month;
    _day = day;
}

int ImageInfo::year() const
{
    return _year;
}

int ImageInfo::month() const
{
    return _month;
}

int ImageInfo::day() const
{
    return _day;
}

void ImageInfo::setTime( int hour, int minute )
{
    _hour = hour;
    _minute = minute;
}

int ImageInfo::hour() const
{
    return _hour;
}

int ImageInfo::minute() const
{
    return _minute;
}

void ImageInfo::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
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

QPixmap ImageInfo::pixmap( int width, int height )
{
    QImage img( _fileName );
    QPixmap pix;
    img = img.smoothScale( width,  height, QImage::ScaleMin );
    pix.convertFromImage( img );
    return pix;
}

QDomElement ImageInfo::save( QDomDocument& doc )
{
    QDomElement elm = doc.createElement( "Image" );
    elm.setAttribute( "file",  QFileInfo( _fileName ).fileName() );
    elm.setAttribute( "label",  _label );
    elm.setAttribute( "description", _description );
    elm.setAttribute( "year", _year );
    elm.setAttribute( "month",  _month );
    elm.setAttribute( "day",  _day );
    elm.setAttribute( "hour",  _hour );
    elm.setAttribute( "minute",  _minute );
    elm.setAttribute( "quality",  _quality );
    for( QMapIterator<QString,QStringList> it= _options.begin(); it != _options.end(); ++it ) {
        QDomElement opt = doc.createElement( "option" );
        opt.setAttribute( "name",  it.key() );
        elm.appendChild( opt );
        QStringList list = it.data();
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( "value" );
            val.setAttribute( "value", *it2 );
            opt.appendChild( val );
        }
    }
    return elm;
}











