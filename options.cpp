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
#include <qregexp.h>

Options* Options::_instance = 0;

Options* Options::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

Options::~Options()
{
}


Options::Options( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QString& imageDirectory )
    : _thumbSize( 64 ), _hasAskedAboutTimeStamps( false ), _imageDirectory( imageDirectory )
{
    _thumbSize = config.attribute( QString::fromLatin1("thumbSize"), QString::number(_thumbSize) ).toInt();
    _tTimeStamps = (TimeStampTrust) config.attribute( QString::fromLatin1("trustTimeStamps"),  QString::fromLatin1("0") ).toInt();
    _autoSave = config.attribute( QString::fromLatin1("autoSave"), QString::number( 5 ) ).toInt();
    _maxImages = config.attribute( QString::fromLatin1("maxImages"), QString::number( 100 ) ).toInt();
    _htmlBaseDir = config.attribute( QString::fromLatin1("htmlBaseDir"), QString::fromLocal8Bit(getenv("HOME")) + QString::fromLatin1("/public_html") );
    _htmlBaseURL = config.attribute( QString::fromLatin1("htmlBaseURL"), QString::fromLatin1( "file://" ) + _htmlBaseDir );
    _infoBoxPosition = (Position) config.attribute( QString::fromLatin1("infoBoxPosition"), QString::fromLatin1("0") ).toInt();
    _showInfoBox = config.attribute( QString::fromLatin1("showInfoBox"), QString::fromLatin1("1") ).toInt();
    _showDrawings = config.attribute( QString::fromLatin1("showDrawings"), QString::fromLatin1("1") ).toInt();
    _showDescription = config.attribute( QString::fromLatin1("showDescription"), QString::fromLatin1("1") ).toInt();
    _showDate = config.attribute( QString::fromLatin1("showDate"), QString::fromLatin1("1") ).toInt();

    Util::readOptions( options, &_options, &_optionGroups );
    _configDock = configWindowSetup;
}

void Options::setThumbSize( int w )
{
    if ( _thumbSize != w )
        emit changed();
    _thumbSize = w;
}

int Options::thumbSize() const
{
    return _thumbSize;
}


void Options::save( QDomElement top )
{
    QDomDocument doc = top.ownerDocument();
    QDomElement config = doc.createElement( QString::fromLatin1( "config" ) );
    top.appendChild( config );

    config.setAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "1" ) );
    config.setAttribute( QString::fromLatin1("thumbSize"), _thumbSize );
    config.setAttribute( QString::fromLatin1("trustTimeStamps"), _tTimeStamps );
    config.setAttribute( QString::fromLatin1("autoSave"), _autoSave );
    config.setAttribute( QString::fromLatin1("maxImages" ), _maxImages );
    config.setAttribute( QString::fromLatin1("imageDirectory"), _imageDirectory );
    config.setAttribute( QString::fromLatin1("htmlBaseDir"), _htmlBaseDir );
    config.setAttribute( QString::fromLatin1("htmlBaseURL"), _htmlBaseURL );

    config.setAttribute( QString::fromLatin1("infoBoxPosition"), (int) _infoBoxPosition );
    config.setAttribute( QString::fromLatin1("showInfoBox"), _showInfoBox );
    config.setAttribute( QString::fromLatin1("showDrawings"), _showDrawings );
    config.setAttribute( QString::fromLatin1("showDescription"), _showDescription );
    config.setAttribute( QString::fromLatin1("showDate"), _showDate );

    QStringList grps = optionGroups();
    QDomElement options = doc.createElement( QString::fromLatin1("options") );
    top.appendChild( options );
    (void) Util::writeOptions( doc, options, _options, &_optionGroups );

    // Save window layout for config window
    top.appendChild( _configDock );
}

void Options::setOption( const QString& key, const QStringList& value )
{
    if ( _options[key] != value )
        emit changed();
    _options[key] = value;
}

void Options::removeOption( const QString& key, const QString& value )
{
    emit changed();
    _options[key].remove( value );
}

void Options::addOption( const QString& key, const QString& value )
{
    emit changed();
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
        emit changed();
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
    if ( _showInfoBox != b ) emit changed();
    _showInfoBox = b;
}

void Options::setShowDrawings(bool b)
{
    if ( _showDrawings != b ) emit changed();
    _showDrawings = b;
}

void Options::setShowDescription(bool b)
{
    if ( _showDescription != b ) emit changed();
    _showDescription = b;
}

void Options::setShowDate(bool b)
{
    if ( _showDate != b ) emit changed();
    _showDate = b;
}

Options::Position Options::infoBoxPosition() const
{
    return _infoBoxPosition;
}

void Options::setInfoBoxPosition( Position pos )
{
    if ( _infoBoxPosition != pos ) emit changed();
    _infoBoxPosition = pos;
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
    if ( _optionGroups[optionGroup]._show != b ) emit changed();
    _optionGroups[optionGroup]._show = b;
}

QString Options::HTMLBaseDir() const
{
    return _htmlBaseDir;
}

void Options::setHTMLBaseDir( const QString& dir )
{
    if ( _htmlBaseDir != dir ) emit changed();
    _htmlBaseDir = dir;
}

QString Options::HTMLBaseURL() const
{
    return _htmlBaseURL;
}

void Options::setHTMLBaseURL( const QString& dir )
{
    if ( _htmlBaseURL != dir ) emit changed();
    _htmlBaseURL = dir;
}

void Options::setAutoSave( int min )
{
    if ( _autoSave != min ) emit changed();
    _autoSave = min;
}

int Options::autoSave() const
{
    return _autoSave;
}

void Options::setMaxImages( int i )
{
    if ( _maxImages != i ) emit changed();
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
    emit changed();
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
    emit changed();

    // No need to remove all the items from the image database for this option group, as this
    // Will happen when saving
    _optionGroups.erase( name );

    emit optionGroupsChanged();
}

void Options::renameOptionGroup( const QString& oldName, const QString& newName )
{
    emit changed();

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
    emit changed();

    _optionGroups[name]._icon = icon;
    emit optionGroupsChanged();
}

void Options::saveConfigWindowLayout( ImageConfig* config )
{
    QDomDocument doc;
    _configDock = doc.createElement( QString::fromLatin1("configWindowSetup" ) );
    config->writeDockConfig( _configDock );
    emit changed();
}


void Options::loadConfigWindowLayout( ImageConfig* config )
{
    config->readDockConfig( _configDock );
}

void Options::setup( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QString& imageDirectory )
{
    _instance = new Options( config, options, configWindowSetup, imageDirectory );
}

#include "options.moc"
