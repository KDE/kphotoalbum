/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kglobal.h>
#include "imagedb.h"
#include "imageconfig.h"
#include <qtextstream.h>

Options* Options::_instance = 0;
QString Options::_confFile = QString::null;

Options* Options::instance()
{
    if ( ! _instance )
        _instance = new Options();
    return _instance;
}


Options::Options()
    : _thumbSize( 64 ), _hasAskedAboutTimeStamps( false ), _dirty( false )
{
    if ( _confFile.isNull() )
        _confFile = QDir::home().path() + QString::fromLatin1("/.kimdaba");

    Util::checkForBackupFile( _confFile, autoSaveFile() );

    QDomDocument doc;
    QFile file( _confFile );
    if ( file.open( IO_ReadOnly ) )
        doc.setContent( &file );
    else {
        QFile file( locate( "data", QString::fromLatin1( "kimdaba/default-setup" ) ) );
        if ( !file.open( IO_ReadOnly ) ) {
            KMessageBox::information( 0, i18n( "KimDaBa was unable to load a default setup, which indicates an installation error" ), i18n("No default setup file found") );
        }
        else {
            QTextStream stream( &file );
            QString str = stream.read();
            str = str.replace( QString::fromLatin1( "Persons" ), i18n( "Persons" ) );
            str = str.replace( QString::fromLatin1( "Locations" ), i18n( "Locations" ) );
            str = str.replace( QString::fromLatin1( "Keywords" ), i18n( "Keywords" ) );
            doc.setContent( str );
        }
    }

    QDomElement top = doc.documentElement();

    _thumbSize = top.attribute( QString::fromLatin1("thumbSize"), QString::number(_thumbSize) ).toInt();
    _tTimeStamps = (TimeStampTrust) top.attribute( QString::fromLatin1("trustTimeStamps"),  QString::fromLatin1("0") ).toInt();
    _autoSave = top.attribute( QString::fromLatin1("autoSave"), QString::number( 5 ) ).toInt();
    _maxImages = top.attribute( QString::fromLatin1("maxImages"), QString::number( 100 ) ).toInt();
    _imageDirectory = top.attribute( QString::fromLatin1("imageDirectory") );
    _htmlBaseDir = top.attribute( QString::fromLatin1("htmlBaseDir"), QString::fromLocal8Bit(getenv("HOME")) + QString::fromLatin1("/public_html") );
    _htmlBaseURL = top.attribute( QString::fromLatin1("htmlBaseURL"), QString::fromLatin1( "file://" ) + _htmlBaseDir );
    _infoBoxPosition = (Position) top.attribute( QString::fromLatin1("infoBoxPosition"), QString::fromLatin1("0") ).toInt();
    _showInfoBox = top.attribute( QString::fromLatin1("showInfoBox"), QString::fromLatin1("1") ).toInt();
    _showDrawings = top.attribute( QString::fromLatin1("showDrawings"), QString::fromLatin1("1") ).toInt();
    _showDescription = top.attribute( QString::fromLatin1("showDescription"), QString::fromLatin1("1") ).toInt();
    _showDate = top.attribute( QString::fromLatin1("showDate"), QString::fromLatin1("1") ).toInt();

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        if ( !node.isElement() )
            continue;
        QDomElement elm = node.toElement();
        QString tag = elm.tagName();
        if ( tag == QString::fromLatin1( "options" ) )
            Util::readOptions( elm, &_options, &_optionGroups );
        else if ( tag == QString::fromLatin1("configWindowSetup") )
            _configDock = elm;
        else
            qWarning("Unknown node: '%s'", tag.latin1() );
    }
}

void Options::setThumbSize( int w )
{
    if ( _thumbSize != w )
        _dirty = true;
    _thumbSize = w;
}

int Options::thumbSize() const
{
    return _thumbSize;
}


void Options::save( const QString& fileName )
{
    QFile file( fileName );
    if ( !file.open( IO_WriteOnly ) )  {
        // PENDING(blackie) Do it the KDE way
        qWarning( "Could't open file %s for writing", fileName.latin1() );
        return;
    }

    QDomDocument doc;
    // PENDING(blackie) The user should be able to specify the coding himself.
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"),
                                                      QString::fromLatin1("version=\"1.0\" "
                                                                          "encoding=\"UTF-8\"" ) ) );
    QDomElement top = doc.createElement( QString::fromLatin1("kimdaba") );
    top.setAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "1" ) );
    doc.appendChild( top );

    top.setAttribute( QString::fromLatin1("thumbSize"), _thumbSize );
    top.setAttribute( QString::fromLatin1("trustTimeStamps"), _tTimeStamps );
    top.setAttribute( QString::fromLatin1("autoSave"), _autoSave );
    top.setAttribute( QString::fromLatin1("maxImages" ), _maxImages );
    top.setAttribute( QString::fromLatin1("imageDirectory"), _imageDirectory );
    top.setAttribute( QString::fromLatin1("htmlBaseDir"), _htmlBaseDir );
    top.setAttribute( QString::fromLatin1("htmlBaseURL"), _htmlBaseURL );

    top.setAttribute( QString::fromLatin1("infoBoxPosition"), (int) _infoBoxPosition );
    top.setAttribute( QString::fromLatin1("showInfoBox"), _showInfoBox );
    top.setAttribute( QString::fromLatin1("showDrawings"), _showDrawings );
    top.setAttribute( QString::fromLatin1("showDescription"), _showDescription );
    top.setAttribute( QString::fromLatin1("showDate"), _showDate );
    QStringList grps = optionGroups();

    QDomElement options = doc.createElement( QString::fromLatin1("options") );
    top.appendChild( options );
    (void) Util::writeOptions( doc, options, _options, &_optionGroups );

    // Save window layout for config window
    top.appendChild( _configDock );

    QTextStream stream( &file );
    stream << doc.toString().utf8();
    file.close();
    _dirty = false;
}

void Options::setOption( const QString& key, const QStringList& value )
{
    if ( _options[key] != value )
        _dirty = true;
    _options[key] = value;
}

void Options::removeOption( const QString& key, const QString& value )
{
    _dirty = true;
    _options[key].remove( value );
}

void Options::addOption( const QString& key, const QString& value )
{
    _dirty = true;
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
    if ( _tTimeStamps != t )
        _dirty = true;
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
    if ( _imageDirectory != directory )
        _dirty = true;

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

void Options::setShowInfoBox(bool b)
{
    if ( _showInfoBox != b ) _dirty = true;
    _showInfoBox = b;
}

void Options::setShowDrawings(bool b)
{
    if ( _showDrawings != b ) _dirty = true;
    _showDrawings = b;
}

void Options::setShowDescription(bool b)
{
    if ( _showDescription != b ) _dirty = true;
    _showDescription = b;
}

void Options::setShowDate(bool b)
{
    if ( _showDate != b ) _dirty = true;
    _showDate = b;
}

Options::Position Options::infoBoxPosition() const
{
    return _infoBoxPosition;
}

void Options::setInfoBoxPosition( Position pos )
{
    if ( _infoBoxPosition != pos ) _dirty = true;
    _infoBoxPosition = pos;
}

void Options::setConfFile( const QString& file )
{
    _confFile = file;
}

/**
   Returns whether the given option groups is shown in the viewer.
*/
bool Options::showOption( const QString& optionGroup ) const
{
    return _optionGroups[optionGroup]._show;
}

void Options::setShowOption( const QString& optionGroup, bool b )
{
    if ( _optionGroups[optionGroup]._show != b ) _dirty = true;
    _optionGroups[optionGroup]._show = b;
}

QString Options::HTMLBaseDir() const
{
    return _htmlBaseDir;
}

void Options::setHTMLBaseDir( const QString& dir )
{
    if ( _htmlBaseDir != dir ) _dirty = true;
    _htmlBaseDir = dir;
}

QString Options::HTMLBaseURL() const
{
    return _htmlBaseURL;
}

void Options::setHTMLBaseURL( const QString& dir )
{
    if ( _htmlBaseURL != dir ) _dirty = true;
    _htmlBaseURL = dir;
}

void Options::setAutoSave( int min )
{
    if ( _autoSave != min ) _dirty = true;
    _autoSave = min;
}

int Options::autoSave() const
{
    return _autoSave;
}

void Options::setMaxImages( int i )
{
    if ( _maxImages != i ) _dirty = true;
    _maxImages = i;
}

int Options::maxImages() const
{
    return _maxImages;
}

/**
   Adds a new option group.
   \param name Name as used in the XML file, and a used as keys for Options::setOption.
   \param label Text label as seen in the GUI
   \param icon to be shown in the browser
*/
void Options::addOptionGroup( const QString& name, const QString& label, const QString& icon )
{
    _dirty = true;
    _optionGroups[name] = OptionGroupInfo(label,icon);
    emit optionGroupsChanged();
}

QStringList Options::optionGroups() const
{
    return QStringList( _optionGroups.keys() );
}

QString Options::textForOptionGroup( const QString& name ) const
{
    return _optionGroups[name]._text;
}

QPixmap Options::iconForOptionGroup( const QString& name ) const
{
    return KGlobal::iconLoader()->loadIcon( _optionGroups[name]._icon, KIcon::Desktop, 22 );
}

QString Options::iconNameForOptionGroup( const QString& name ) const
{
    return _optionGroups[name]._icon;
}

void Options::deleteOptionGroup( const QString& name )
{
    _dirty = true;

    // No need to remove all the items from the image database for this option group, as this
    // Will happen when saving
    _optionGroups.erase( name );

    emit optionGroupsChanged();
}

void Options::renameOptionGroup( const QString& oldName, const QString& newName )
{
    _dirty = true;

    _optionGroups[newName] = _optionGroups[oldName];
    _optionGroups.erase( oldName );
    _optionGroups[newName]._text = newName;
    _options[newName] = _options[oldName];
    _options.erase(oldName);

    ImageDB::instance()->renameOptionGroup( oldName, newName );
    emit optionGroupsChanged();
}

void Options::setIconForOptionGroup( const QString& name, const QString& icon )
{
    _dirty = true;

    _optionGroups[name]._icon = icon;
    emit optionGroupsChanged();
}

QString Options::configFile() const
{
    return _confFile;
}

QString Options::autoSaveFile() const
{
    QFileInfo fi( _confFile );
    QString file = fi.fileName();
    QString path = fi.dirPath();
    return path + QString::fromLatin1( "/.#" ) + file;
}

void Options::saveConfigWindowLayout( ImageConfig* config )
{
    QDomDocument doc;
    _configDock = doc.createElement( QString::fromLatin1("configWindowSetup" ) );
    config->writeDockConfig( _configDock );
    _dirty = true;
}


void Options::loadConfigWindowLayout( ImageConfig* config )
{
    config->readDockConfig( _configDock );
}

#include "options.moc"
