#include "options.h"
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

Options* Options::_instance = 0;

Options* Options::instance()
{
    if ( ! _instance )
        _instance = new Options();
    return _instance;
}


Options::Options()
    : _thumbWidth( 32 ),  _thumbHeight( 32 ),  _numThreads( 1 ),  _cacheThumbNails( true ),  _use4To3Ratio( true )
{
    QFile file( QDir::home().path() + "/.kpalbum" );
    if ( !file.open( IO_ReadOnly ) )  {
        qWarning("No ~/.kpalbum found");
        return;
    }

    QDomDocument doc;
    doc.setContent( &file );
    QDomElement top = doc.documentElement();

    _thumbWidth = top.attribute( "thumbWidth", QString::number(_thumbWidth) ).toInt();
    _thumbHeight = top.attribute( "thumbHeight",  QString::number( _thumbHeight ) ).toInt();
    _numThreads = top.attribute( "numThreads",  QString::number( _numThreads ) ).toInt();
    _cacheThumbNails = top.attribute( "cacheThumbNails",  QString::number( _cacheThumbNails ) ).toInt();
    _use4To3Ratio = top.attribute( "use4To3Ratio",  QString::number( _use4To3Ratio ) ).toInt();
    _trustTimeStamps = top.attribute( "trustTimeStamps",  "1" ).toInt();
    _trustDateStamps = top.attribute( "trustDateStamps",  "1" ).toInt();
}

void Options::setThumbWidth( int w )
{
    _thumbWidth = w;
}

int Options::thumbWidth() const
{
    return _thumbWidth;
}

void Options::setThumbHeight( int h )
{
    _thumbHeight = h;
}

int Options::thumbHeight() const
{
    if ( _use4To3Ratio )
        return 3*_thumbWidth/4;

    return _thumbHeight;
}

void Options::setNumThreads( int count )
{
    _numThreads = count;
}

int Options::numThreads() const
{
    return _numThreads;
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

    top.setAttribute( "thumbWidth", _thumbWidth );
    top.setAttribute( "thumbHeight", _thumbHeight );
    top.setAttribute( "numThreads", _numThreads );
    top.setAttribute( "cacheThumbNails", _cacheThumbNails );
    top.setAttribute( "use4To3Ratio", _use4To3Ratio );
    top.setAttribute( "trustTimeStamps", _trustTimeStamps );
    top.setAttribute( "trustDateStamps", _trustDateStamps );

    QTextStream stream( &file );
    stream << doc.toString();
    file.close();
}

void Options::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
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

void Options::setTrustDateStamps( bool b)
{
    _trustDateStamps = b;
}

bool Options::trustDateStamps() const
{
    return _trustDateStamps;
}



