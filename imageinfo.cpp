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

void ImageInfo::setDateFrom( int year, int month, int day )
{
    _yearFrom = year;
    _monthFrom = month;
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

void ImageInfo::setDateTo( int year, int month, int day )
{
    _yearTo = year;
    _monthTo = month;
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

    elm.setAttribute( "yearFrom", _yearFrom );
    elm.setAttribute( "monthFrom",  _monthFrom );
    elm.setAttribute( "dayFrom",  _dayFrom );

    elm.setAttribute( "yearTo", _yearTo );
    elm.setAttribute( "monthTo",  _monthTo );
    elm.setAttribute( "dayTo",  _dayTo );

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











