#include "options.h"
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "util.h"
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qapplication.h>
#include <qcursor.h>

Options* Options::_instance = 0;
QString Options::_confFile = QString::null;

Options* Options::instance()
{
    if ( ! _instance )
        _instance = new Options();
    return _instance;
}


Options::Options()
    : _thumbSize( 64 ), _cacheThumbNails( true ), _hasAskedAboutTimeStamps( false )
{
    if ( _confFile.isNull() )
        _confFile = QDir::home().path() + "/.kpalbum";

    QDomDocument doc;
    QFile file( _confFile );
    if ( file.open( IO_ReadOnly ) )
        doc.setContent( &file );
    else
        doc.setContent( QString("<Options>") );

    QDomElement top = doc.documentElement();

    _thumbSize = top.attribute( "thumbSize", QString::number(_thumbSize) ).toInt();
    _cacheThumbNails = top.attribute( "cacheThumbNails",  QString::number( _cacheThumbNails ) ).toInt();
    _tTimeStamps = (TimeStampTrust) top.attribute( "trustTimeStamps",  "0" ).toInt();
    _autoSave = top.attribute( "autoSave", QString::number( 5 ) ).toInt();
    _imageDirectory = top.attribute( "imageDirectory" );
    _htmlBaseDir = top.attribute( "htmlBaseDir", QString::fromLocal8Bit(getenv("HOME")) + QString::fromLatin1("/public_html") );
    _htmlBaseURL = top.attribute( "htmlBaseURL", QString::fromLatin1( "file://" ) + _htmlBaseDir );
    _infoBoxPosition = (Position) top.attribute( "infoBoxPosition", "0" ).toInt();
    _showInfoBox = top.attribute( "showInfoBox", "1" ).toInt();
    _showDrawings = top.attribute( "showDrawings", "1" ).toInt();
    _showDescription = top.attribute( "showDescription", "1" ).toInt();
    _showDate = top.attribute( "showDate", "1" ).toInt();
    _showNames = top.attribute( "showNames", "0" ).toInt();
    _showLocation = top.attribute( "showLocation", "0" ).toInt();
    _showKeyWords = top.attribute( "showKeyWords", "0" ).toInt();

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

QStringList Options::dataDirs() const
{
    return QStringList() << "/home/blackie/Images/take1";
}

void Options::save()
{
    QFile file( _confFile );
    if ( !file.open( IO_WriteOnly ) )  {
        qWarning( "Could't open file %s for writting", _confFile.latin1() );
        return;
    }

    QDomDocument doc;
    // PENDING(blackie) The user should be able to specify the coding himself.
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
    doc.appendChild( doc.createElement( "Options" ) );
    QDomElement top = doc.documentElement();

    top.setAttribute( "thumbSize", _thumbSize );
    top.setAttribute( "cacheThumbNails", _cacheThumbNails );
    top.setAttribute( "trustTimeStamps", _tTimeStamps );
    top.setAttribute( "autoSave", _autoSave );
    top.setAttribute( "imageDirectory", _imageDirectory );
    top.setAttribute( "htmlBaseDir", _htmlBaseDir );
    top.setAttribute( "htmlBaseURL", _htmlBaseURL );

    top.setAttribute( "infoBoxPosition", (int) _infoBoxPosition );
    top.setAttribute( "showInfoBox", _showInfoBox );
    top.setAttribute( "showDrawings", _showDrawings );
    top.setAttribute( "showDescription", _showDescription );
    top.setAttribute( "showDate", _showDate );
    top.setAttribute( "showNames", _showNames );
    top.setAttribute( "showLocation", _showLocation );
    top.setAttribute( "showKeyWords", _showKeyWords );

    (void) Util::writeOptions( doc, top, _options );

    QTextStream stream( &file );
    stream << doc.toString().utf8();
    file.close();
}

void Options::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
}

void Options::removeOption( const QString& key, const QString& value )
{
    _options[key].remove( value );
}

void Options::addOption( const QString& key, const QString& value )
{
    if ( _options[key].contains( value ) )
        _options[key].remove( value );
    _options[key].prepend( value );
}

QStringList Options::optionValue( const QString& key ) const
{
    return _options[key];
}

bool Options::trustTimeStamps()
{
    if ( _tTimeStamps == Always )
        return true;
    else if ( _tTimeStamps == Never )
        return false;
    else {
        if (!_hasAskedAboutTimeStamps ) {
            QApplication::setOverrideCursor( Qt::ArrowCursor );
            int answer = KMessageBox::questionYesNo( 0, i18n("New images was found. Should I trust their time stamps?"),
                                                     i18n("Trust Time Stamps") );
            QApplication::restoreOverrideCursor();
            if ( answer == KMessageBox::Yes )
                _trustTimeStamps = true;
            else
                _trustTimeStamps = false;
            _hasAskedAboutTimeStamps = true;
        }
        return _trustTimeStamps;
    }
}
void Options::setTTimeStamps( TimeStampTrust t )
{
    _tTimeStamps = t;
}

Options::TimeStampTrust Options::tTimeStamps() const
{
    return _tTimeStamps;
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

bool Options::showDrawings() const
{
    return _showDrawings;
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

void Options::setShowDrawings(bool b)
{
    _showDrawings = b;
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

bool Options::showKeyWords() const
{
    return _showKeyWords;
}

void Options::setShowKeyWords( bool b )
{
    _showKeyWords = b;
}

QString Options::HTMLBaseDir() const
{
    return _htmlBaseDir;
}

void Options::setHTMLBaseDir( const QString& dir )
{
    _htmlBaseDir = dir;
}

QString Options::HTMLBaseURL() const
{
    return _htmlBaseURL;
}

void Options::setHTMLBaseURL( const QString& dir )
{
    _htmlBaseURL = dir;
}

void Options::setAutoSave( int min )
{
    _autoSave = min;
}

int Options::autoSave() const
{
    return _autoSave;
}

