/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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
#include <qmessagebox.h>
#include "categorycollection.h"
#include <qdatetime.h>
#include <qnamespace.h>
#include "imageinfo.h"
#include <kapplication.h>
#include <kconfig.h>
#include "options.moc"

Options* Options::_instance = 0;

Options* Options::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

Options::Options( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory )
    : _hasAskedAboutTimeStamps( false ), _imageDirectory( imageDirectory )
{
    _htmlBaseDir = config.attribute( QString::fromLatin1("htmlBaseDir"), QString::fromLocal8Bit(getenv("HOME")) + QString::fromLatin1("/public_html") );
    _htmlBaseURL = config.attribute( QString::fromLatin1("htmlBaseURL"), QString::fromLatin1( "file://" ) + _htmlBaseDir );
    _htmlDestURL = config.attribute( QString::fromLatin1("htmlDestURL"), QString::fromLatin1( "file://" ) + _htmlBaseDir );

    _locked = config.attribute( QString::fromLatin1( "locked" ), QString::fromLatin1( "0" ) ).toInt();
    _exclude = config.attribute( QString::fromLatin1( "exclude" ), QString::fromLatin1( "1" ) ).toInt();
    _passwd = config.attribute( QString::fromLatin1( "passwd" ) );

    Util::readOptions( options, &_options, CategoryCollection::instance() );
    createSpecialCategories();

    _members.load( memberGroups );
    _currentLock.load( config );
}

void Options::save( QDomElement top )
{
    QDomDocument doc = top.ownerDocument();
    QDomElement config = doc.createElement( QString::fromLatin1( "config" ) );
    top.appendChild( config );

    config.setAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "1" ) );
    config.setAttribute( QString::fromLatin1("imageDirectory"), _imageDirectory );

    config.setAttribute( QString::fromLatin1("htmlBaseDir"), _htmlBaseDir );
    config.setAttribute( QString::fromLatin1("htmlBaseURL"), _htmlBaseURL );
    config.setAttribute( QString::fromLatin1("htmlDestURL"), _htmlDestURL );

    config.setAttribute( QString::fromLatin1("locked"), _locked );
    config.setAttribute( QString::fromLatin1("exclude"), _exclude );
    config.setAttribute( QString::fromLatin1("passwd"), _passwd );

    QStringList grps = CategoryCollection::instance()->categoryNames();
    QDomElement options = doc.createElement( QString::fromLatin1("options") );
    top.appendChild( options );
    (void) Util::writeOptions( doc, options, _options, CategoryCollection::instance() );

    // Member Groups
    if ( ! _members.isEmpty() )
        top.appendChild( _members.save( doc ) );

    if ( !_currentLock.isNull() )
        config.appendChild( _currentLock.toXML( doc ) );
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
    emit deletedOption( key, value );
}

void Options::renameOption( const QString& category, const QString& oldValue, const QString& newValue )
{
    _options[category].remove( oldValue );
    addOption( category, newValue );
    emit renamedOption( category, oldValue, newValue );
}

void Options::addOption( const QString& key, const QString& value )
{
    if ( _options[key].contains( value ) )
        _options[key].remove( value );
    else
        emit changed();
    _options[key].prepend( value );
}

QStringList Options::optionValue( const QString& key ) const
{
    return _options[key];
}

QStringList Options::optionValueInclGroups( const QString& category ) const
{
    // values including member groups

    QStringList items = optionValue( category );
    QStringList itemsAndGroups = QStringList::QStringList();
    for( QStringList::Iterator it = items.begin(); it != items.end(); ++it ) {
        itemsAndGroups << *it ;
    };
    // add the groups to the listbox too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = _members.groups( category );
    for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        if ( ! items.contains(  *it ) )
            itemsAndGroups << *it ;
    };
    if ( viewSortType() == SortAlpha )
        itemsAndGroups.sort();
    return itemsAndGroups;
}


bool Options::trustTimeStamps()
{
    if ( tTimeStamps() == Always )
        return true;
    else if ( tTimeStamps() == Never )
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
    setValue( "General", "trustTimeStamps", (int) t );
}

Options::TimeStampTrust Options::tTimeStamps() const
{
    return (TimeStampTrust) value(  "General", "trustTimeStamps", (int) Always );
}

QString Options::imageDirectory() const
{
    if ( !_imageDirectory.endsWith( QString::fromLatin1( "/" ) ) )
        return _imageDirectory + QString::fromLatin1( "/" );
    else
        return _imageDirectory;
}


Options::Position Options::infoBoxPosition() const
{
    return (Position) value( "Viewer", "infoBoxPosition", 0 );
}

void Options::setInfoBoxPosition( Position pos )
{
    setValue( "Viewer", "infoBoxPosition", (int) pos );
}

/**
   Returns whether the given category is shown in the viewer.
*/
bool Options::showOption( const QString& category ) const
{
    return CategoryCollection::instance()->categoryForName(category)->doShow();
}

void Options::setShowOption( const QString& category, bool b )
{
    if ( CategoryCollection::instance()->categoryForName(category)->doShow() != b ) emit changed();
    CategoryCollection::instance()->categoryForName(category)->setDoShow( b );
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

void Options::setHTMLBaseURL( const QString& url )
{
    if ( _htmlBaseURL != url ) emit changed();
    _htmlBaseURL = url;
}

QString Options::HTMLDestURL() const
{
    return _htmlDestURL;
}

void Options::setHTMLDestURL( const QString& url )
{
    if ( _htmlDestURL != url ) emit changed();
    _htmlDestURL = url;
}


void Options::setup( const QDomElement& config, const QDomElement& options,
                     const QDomElement& configWindowSetup, const QDomElement& memberGroups,
                     const QString& imageDirectory )
{
    _instance = new Options( config, options, configWindowSetup, memberGroups, imageDirectory );
}

const MemberMap& Options::memberMap()
{
    return _members;
}

void Options::setMemberMap( const MemberMap& members )
{
    // In a perfect world, I should check if _members != members, and only emit changed in that case.
    emit changed();
    _members = members;
}

void Options::setCurrentLock( const ImageSearchInfo& info, bool exclude )
{
    _currentLock = info;
    _exclude = exclude;
}

ImageSearchInfo Options::currentLock() const
{
    return _currentLock;
}

void Options::setLocked( bool lock )
{
    _locked = lock;
    emit locked( lock, _exclude );
}

bool Options::isLocked() const
{
    return _locked;
}

bool Options::lockExcludes() const
{
    return _exclude;
}

void Options::setPassword( const QString& passwd )
{
    _passwd = passwd;
}

QString Options::password() const
{
    return _passwd;
}

QString Options::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + QString::fromLatin1("CategoryImages" );
    member.replace( ' ', '_' );
    QString fileName = dir + QString::fromLatin1("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}


void Options::setOptionImage( const QString& category, QString member, const QImage& image )
{
    QString dir = imageDirectory() + QString::fromLatin1("CategoryImages" );
    QFileInfo fi( dir );
    bool ok;
    if ( !fi.exists() ) {
        bool ok = QDir().mkdir( dir );
        if ( !ok ) {
            QMessageBox::warning( 0, i18n("Unable to Create Directory"), i18n("Unable to create directory '%1'.").arg( dir ), QMessageBox::Ok, 0 );
            return;
        }
    }
    QString fileName = fileForCategoryImage( category, member );
    ok = image.save( fileName, "JPEG" );
    if ( !ok ) {
        QMessageBox::warning( 0, i18n("Error Saving Image"), i18n("Error when saving image '%1'.").arg(fileName), QMessageBox::Ok, 0 );
        return;
    }
}

QImage Options::optionImage( const QString& category, QString member, int size ) const
{
    QString fileName = fileForCategoryImage( category, member );
    QImage img;
    bool ok = img.load( fileName, "JPEG" );
    if ( ! ok ) {
        if ( Options::instance()->memberMap().isGroup( category, member ) )
            img = KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kuser" ), KIcon::Desktop, size );
        else
            img = CategoryCollection::instance()->categoryForName( category )->icon( size );
    }
    return img.smoothScale( size, size, QImage::ScaleMin );
}

void Options::setViewSortType( ViewSortType tp )
{
    bool changed = ( viewSortType() != tp );
    setValue( "General", "viewSortType", (int) tp );
    if ( changed )
        emit viewSortTypeChanged( tp );
}

Options::ViewSortType Options::viewSortType() const
{
    return (ViewSortType) value( "General", "viewSortType", 0 );
}

void Options::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "fromDate", date.toString( Qt::ISODate ) );
}

QDate Options::fromDate() const
{
    QString date = value("Miscellaneous", "fromDate", "" );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year(), 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

void  Options::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( "Miscellaneous", "toDate", date.toString( Qt::ISODate ) );
}

QDate Options::toDate() const
{
    QString date = value("Miscellaneous", "toDate", "" );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year()+1, 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

QString Options::albumCategory() const
{
    QString category = value( "General", "albumCategory", "" );

    if ( !CategoryCollection::instance()->categoryNames().contains( category ) ) {
        category = CategoryCollection::instance()->categoryNames()[0];
        const_cast<Options*>(this)->setAlbumCategory( category );
    }

    return category;
}

void Options::setAlbumCategory( const QString& category )
{
    setValue( "General", "albumCategory", category );
}

void Options::setWindowGeometry( WindowType win, const QRect& geometry )
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    config->writeEntry( windowTypeToString( win ), geometry );
}

QRect Options::windowGeometry( WindowType win ) const
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    QRect rect( 0,0, 800, 600 );
    return config->readRectEntry( windowTypeToString( win ), &rect );
}

bool Options::ready()
{
    return _instance != 0;
}

void Options::createSpecialCategories()
{
    Category* folderCat = CategoryCollection::instance()->categoryForName( QString::fromLatin1( "Folder" ) );
    if( folderCat == 0 ) {
        _options.insert( QString::fromLatin1("Folder"), QStringList() );
        folderCat = new Category( QString::fromLatin1("Folder"), QString::fromLatin1("folder"), Category::Small, Category::ListView, false );
        CategoryCollection::instance()->addCategory( folderCat );
    }
    folderCat->setSpecialCategory( true );


    Category* tokenCat = CategoryCollection::instance()->categoryForName( QString::fromLatin1( "Tokens" ) );
    if ( !tokenCat ) {
        _options.insert( QString::fromLatin1("Tokens"), QStringList() );
        tokenCat = new Category( QString::fromLatin1("Tokens"), QString::fromLatin1("cookie"), Category::Small, Category::ListView, true );
        CategoryCollection::instance()->addCategory( tokenCat );
    }
    tokenCat->setSpecialCategory( true );
}


int Options::value( const char* group, const char* option, int defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readNumEntry( option, defaultValue );
}

QString Options::value( const char* group, const char* option, const char* defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readEntry( option, QString::fromLatin1(defaultValue) );
}

bool Options::value( const char* group, const char* option, bool defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readBoolEntry( option, defaultValue );
}

QColor Options::value( const char* group, const char* option, const QColor& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readColorEntry( option, &defaultValue );
}

QSize Options::value( const char* group, const char* option, const QSize& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readSizeEntry( option, &defaultValue );
}

void Options::setValue( const char* group, const char* option, int value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const char* group, const char* option, const QString& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const char* group, const char* option, bool value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const char* group, const char* option, const QColor& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const char* group, const char* option, const QSize& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

QSize Options::histogramSize() const
{
    return value( "General", "histogramSize", QSize( 15, 30 ) );
}

void Options::setHistogramSize( const QSize& size )
{
    bool changed = (size != histogramSize() );
    setValue( "General", "histogramSize", size );
    if (changed)
        emit histogramSizeChanged( size );
}

const char* Options::windowTypeToString( WindowType tp ) const
{
    switch (tp) {
    case MainWindow: return "MainWindow";
    case ConfigWindow: return "ConfigWindow";
    }
    return "";
}


