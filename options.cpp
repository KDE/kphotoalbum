#include "options.h"
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "util.h"

Options* Options::_instance = 0;
QString Options::_confFile = QString::null;

Options* Options::instance()
{
    if ( ! _instance )
        _instance = new Options();
    return _instance;
}


Options::Options()
    : _thumbSize( 64 ), _cacheThumbNails( true ),  _use4To3Ratio( true )
{
    if ( _confFile.isNull() )
        _confFile = QDir::home().path() + "/.kpalbum";

    QFile file( _confFile );
    if ( !file.open( IO_ReadOnly ) )  {
        return;
    }

    QDomDocument doc;
    doc.setContent( &file );
    QDomElement top = doc.documentElement();

    _thumbSize = top.attribute( "thumbSize", QString::number(_thumbSize) ).toInt();
    _cacheThumbNails = top.attribute( "cacheThumbNails",  QString::number( _cacheThumbNails ) ).toInt();
    _use4To3Ratio = top.attribute( "use4To3Ratio",  QString::number( _use4To3Ratio ) ).toInt();
    _trustTimeStamps = top.attribute( "trustTimeStamps",  "1" ).toInt();
    _imageDirectory = top.attribute( "imageDirectory" );

    _infoBoxPosition = (Position) top.attribute( "infoBoxPosition", "0" ).toInt();
    _showInfoBox = top.attribute( "showInfoBox", "1" ).toInt();
    _showDescription = top.attribute( "showDescription", "1" ).toInt();
    _showDate = top.attribute( "showDate", "1" ).toInt();
    _showNames = top.attribute( "showNames", "0" ).toInt();
    _showLocation = top.attribute( "showLocation", "0" ).toInt();

    Util::readOptions( top, &_options );
}

void Options::setThumbSize( int w )
{
    _thumbSize = w;
}

int Options::thumbSize() const
{
    return _thumbSize;
}

void Options::setCacheThumbNails( bool b )
{
    _cacheThumbNails = b;
}

bool Options::cacheThumbNails() const
{
    return _cacheThumbNails;
}

void Options::setUse4To3Ratio( bool b )
{
    _use4To3Ratio = b;
}

bool Options::use4To3Ratio() const
{
    return _use4To3Ratio;
}

QStringList Options::dataDirs() const
{
    return QStringList() << "/home/blackie/Images/take1";
}

void Options::save()
{
    QFile file( _confFile );
    if ( !file.open( IO_WriteOnly ) )  {
        qWarning( "Could't open file %s", _confFile.latin1() );
        return;
    }

    QDomDocument doc;
    doc.setContent( QString("<Options/>") );
    QDomElement top = doc.documentElement();

    top.setAttribute( "thumbSize", _thumbSize );
    top.setAttribute( "cacheThumbNails", _cacheThumbNails );
    top.setAttribute( "use4To3Ratio", _use4To3Ratio );
    top.setAttribute( "trustTimeStamps", _trustTimeStamps );
    top.setAttribute( "imageDirectory", _imageDirectory );


    top.setAttribute( "infoBoxPosition", (int) _infoBoxPosition );
    top.setAttribute( "showInfoBox", _showInfoBox );
    top.setAttribute( "showDescription", _showDescription );
    top.setAttribute( "showDate", _showDate );
    top.setAttribute( "showNames", _showNames );
    top.setAttribute( "showLocation", _showLocation );

    Util::writeOptions( doc, top, _options );

    QTextStream stream( &file );
    stream << doc.toString();
    file.close();
}

void Options::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
}

void Options::addOption( const QString& key, const QString& value )
{
    _options[key] += value;
}

QStringList Options::optionValue( const QString& key ) const
{
    return _options[key];
}

void Options::setTrustTimeStamps( bool b)
{
    _trustTimeStamps = b;
}

bool Options::trustTimeStamps() const
{
    return _trustTimeStamps;
}

QString Options::imageDirectory() const
{
    return _imageDirectory;
}

void Options::setImageDirecotry( const QString& directory )
{
    _imageDirectory = directory;
}

bool Options::configFileExists()
{
    if ( _confFile.isNull() )
        _confFile = QDir::home().path() + "/.kpalbum";

    QFileInfo info( _confFile );
    return info.exists();
}

bool Options::showInfoBox() const
{
    return _showInfoBox;
}

bool Options::showDescription() const
{
    return _showDescription;
}

bool Options::showDate() const
{
    return _showDate;
}

bool Options::showLocation() const
{
    return _showLocation;
}

bool Options::showNames() const
{
    return _showNames;
}

void Options::setShowInfoBox(bool b)
{
    _showInfoBox = b;
}

void Options::setShowDescription(bool b)
{
    _showDescription = b;
}

void Options::setShowDate(bool b)
{
    _showDate = b;
}

void Options::setShowLocation(bool b)
{
    _showLocation = b;
}

void Options::setShowNames(bool b)
{
    _showNames = b;
}

Options::Position Options::infoBoxPosition() const
{
    return _infoBoxPosition;
}

void Options::setInfoBoxPosition( Position pos )
{
    _infoBoxPosition = pos;
}

void Options::setConfFile( const QString& file )
{
    _confFile = file;
}

