#include "imageinfo.h"
#include <qfileinfo.h>
#include <qimage.h>
#include <qdom.h>
#include "options.h"
#include "util.h"


ImageInfo::ImageInfo()
{
}

ImageInfo::ImageInfo( const QString& fileName )
    : _fileName( fileName ), _visible( true ), _imageOnDisk( true )
{
    QFileInfo fi( Options::instance()->imageDirectory()+ "/" + fileName );
    _label = fi.baseName();
    _angle = 0;

    if ( Options::instance()->trustTimeStamps() )  {
        QDate date = fi.created().date();
        _startDate.setYear(date.year());
        _startDate.setMonth(date.month());
        _startDate.setDay(date.day());
    }
}

ImageInfo::ImageInfo( const QString& fileName, QDomElement elm )
    : _fileName( fileName ), _visible( true )
{
    QFileInfo fi( Options::instance()->imageDirectory()+ "/" + fileName );
    _imageOnDisk = fi.exists();
    _label = elm.attribute( "label",  _label );
    _description = elm.attribute( "description" );

    int yearFrom = 0, monthFrom = 0,  dayFrom = 0, yearTo = 0, monthTo = 0,  dayTo = 0;

    _startDate.setYear( elm.attribute( "yearFrom", QString::number( yearFrom) ).toInt() );
    _startDate.setMonth( elm.attribute( "monthFrom", QString::number(monthFrom) ).toInt() );
    _startDate.setDay( elm.attribute( "dayFrom", QString::number(dayFrom) ).toInt() );

    _endDate.setYear( elm.attribute( "yearTo", QString::number(yearTo) ).toInt() );
    _endDate.setMonth( elm.attribute( "monthTo", QString::number(monthTo) ).toInt() );
    _endDate.setDay( elm.attribute( "dayTo", QString::number(dayTo) ).toInt() );

    _angle = elm.attribute( "angle", "0" ).toInt();
    for ( QDomNode child = elm.firstChild(); !child.isNull(); child = child.nextSibling() ) {
        if ( child.isElement() ) {
            QDomElement childElm = child.toElement();
            if ( childElm.tagName() == QString::fromLatin1( "Options" ) ) {
                Util::readOptions( childElm, &_options );
            }
            else if ( childElm.tagName() == QString::fromLatin1( "Drawings" ) ) {
                _drawList.load( childElm );
            }
            else {
                qWarning("Ups unknown tag '%s'", childElm.tagName().latin1() );
                // PENDING(blackie) Do it the KDE way.
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


void ImageInfo::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
}

void ImageInfo::addOption( const QString& key, const QStringList& value )
{
    for( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
        if (! _options[key].contains( *it ) )
            _options[key] += *it;
    }
}

bool ImageInfo::hasOption( const QString& key, const QString& value )
{
    return _options[key].contains(value);
}

QStringList ImageInfo::optionValue( const QString& key ) const
{
    return _options[key];
}

void ImageInfo::renameOption( const QString& key, const QString& oldValue, const QString& newValue )
{
    QStringList& list = _options[key];
    QStringList::Iterator it = list.find( oldValue );
    if ( it != list.end() ) {
        list.remove( it );
        list.append( newValue );
    }
}

QString ImageInfo::fileName( bool relative )
{
    if (relative)
        return _fileName;
    else
        return Options::instance()->imageDirectory() + "/" + _fileName;
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

    elm.setAttribute( "angle",  _angle );

    if ( _options.count() != 0 ) {
        QDomElement top = doc.createElement( QString::fromLatin1("Options") );
        bool any = Util::writeOptions( doc, top, _options );
        if ( any )
            elm.appendChild( top );
    }

    _drawList.save( doc, elm );
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

void ImageInfo::setVisible( bool b )
{
    _visible = b;
}

bool ImageInfo::visible() const
{
    return _visible;
}


bool ImageInfo::operator!=( const ImageInfo& other )
{
    return !(*this == other);
}

bool ImageInfo::operator==( const ImageInfo& other )
{
    bool changed =
        ( _fileName != other._fileName ||
          _label != other._label ||
          _description != other._description ||
          _startDate != other._startDate ||
          _endDate != other._endDate ||
          _angle != other._angle);
    if ( !changed ) {
        QStringList keys = _options.keys();
        for( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
            changed |= _options[*it] != other._options[*it];
        }
    }
    return !changed;
}

void ImageInfo::removeOption( const QString& key, const QString& value )
{
    _options[key].remove( value );
}

DrawList ImageInfo::drawList() const
{
    return _drawList;
}

void ImageInfo::setDrawList( const DrawList& list )
{
    _drawList = list;
}
