#include "options.h"
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "util.h"

Options* Options::_instance = 0;

Options* Options::instance()
{
    if ( ! _instance )
        _instance = new Options();
    return _instance;
}


Options::Options()
    : _thumbSize( 64 ), _cacheThumbNails( true ),  _use4To3Ratio( true )
{
    QFile file( QDir::home().path() + "/.kpalbum" );
    if ( !file.open( IO_ReadOnly ) )  {
        qWarning("No ~/.kpalbum found");
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
    QFile file( QDir::home().path() + "/.kpalbum" );
    if ( !file.open( IO_WriteOnly ) )  {
        qWarning( "Could't open file ~/.kpalbum" );
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


