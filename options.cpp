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
        _confFile = QDir::home().path() + QString::fromLatin1("/.kimdaba");

    QDomDocument doc;
    QFile file( _confFile );
    if ( file.open( IO_ReadOnly ) )
        doc.setContent( &file );
    else
        doc.setContent( QString::fromLatin1("<Options>") );

    QDomElement top = doc.documentElement();

    _thumbSize = top.attribute( QString::fromLatin1("thumbSize"), QString::number(_thumbSize) ).toInt();
    _cacheThumbNails = top.attribute( QString::fromLatin1("cacheThumbNails"),  QString::number( _cacheThumbNails ) ).toInt();
    _tTimeStamps = (TimeStampTrust) top.attribute( QString::fromLatin1("trustTimeStamps"),  QString::fromLatin1("0") ).toInt();
    _autoSave = top.attribute( QString::fromLatin1("autoSave"), QString::number( 5 ) ).toInt();
    _imageDirectory = top.attribute( QString::fromLatin1("imageDirectory") );
    _htmlBaseDir = top.attribute( QString::fromLatin1("htmlBaseDir"), QString::fromLocal8Bit(getenv("HOME")) + QString::fromLatin1("/public_html") );
    _htmlBaseURL = top.attribute( QString::fromLatin1("htmlBaseURL"), QString::fromLatin1( "file://" ) + _htmlBaseDir );
    _infoBoxPosition = (Position) top.attribute( QString::fromLatin1("infoBoxPosition"), QString::fromLatin1("0") ).toInt();
    _showInfoBox = top.attribute( QString::fromLatin1("showInfoBox"), QString::fromLatin1("1") ).toInt();
    _showDrawings = top.attribute( QString::fromLatin1("showDrawings"), QString::fromLatin1("1") ).toInt();
    _showDescription = top.attribute( QString::fromLatin1("showDescription"), QString::fromLatin1("1") ).toInt();
    _showDate = top.attribute( QString::fromLatin1("showDate"), QString::fromLatin1("1") ).toInt();
    _showNames = top.attribute( QString::fromLatin1("showNames"), QString::fromLatin1("0") ).toInt();
    _showLocation = top.attribute( QString::fromLatin1("showLocation"), QString::fromLatin1("0") ).toInt();
    _showKeyWords = top.attribute( QString::fromLatin1("showKeyWords"), QString::fromLatin1("0") ).toInt();

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

void Options::save()
{
    QFile file( _confFile );
    if ( !file.open( IO_WriteOnly ) )  {
        qWarning( "Could't open file %s for writting", _confFile.latin1() );
        return;
    }

    QDomDocument doc;
    // PENDING(blackie) The user should be able to specify the coding himself.
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"" ) ) );
    doc.appendChild( doc.createElement( QString::fromLatin1("Options") ) );
    QDomElement top = doc.documentElement();

    top.setAttribute( QString::fromLatin1("thumbSize"), _thumbSize );
    top.setAttribute( QString::fromLatin1("cacheThumbNails"), _cacheThumbNails );
    top.setAttribute( QString::fromLatin1("trustTimeStamps"), _tTimeStamps );
    top.setAttribute( QString::fromLatin1("autoSave"), _autoSave );
    top.setAttribute( QString::fromLatin1("imageDirectory"), _imageDirectory );
    top.setAttribute( QString::fromLatin1("htmlBaseDir"), _htmlBaseDir );
    top.setAttribute( QString::fromLatin1("htmlBaseURL"), _htmlBaseURL );

    top.setAttribute( QString::fromLatin1("infoBoxPosition"), (int) _infoBoxPosition );
    top.setAttribute( QString::fromLatin1("showInfoBox"), _showInfoBox );
    top.setAttribute( QString::fromLatin1("showDrawings"), _showDrawings );
    top.setAttribute( QString::fromLatin1("showDescription"), _showDescription );
    top.setAttribute( QString::fromLatin1("showDate"), _showDate );
    top.setAttribute( QString::fromLatin1("showNames"), _showNames );
    top.setAttribute( QString::fromLatin1("showLocation"), _showLocation );
    top.setAttribute( QString::fromLatin1("showKeyWords"), _showKeyWords );

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
            int answer = KMessageBox::questionYesNo( 0, i18n("New images were found. Should I trust their time stamps?"),
                                                     i18n("Trust Time Stamps?") );
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
        _confFile = QDir::home().path() + QString::fromLatin1("/.kimdaba");

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

