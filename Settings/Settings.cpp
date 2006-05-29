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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Settings.h"
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "Utilities/Util.h"
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qapplication.h>
#include <qcursor.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kglobal.h>
#include "DB/ImageDB.h"
#include <qtextstream.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include "DB/CategoryCollection.h"
#include <qdatetime.h>
#include <qnamespace.h>
#include "DB/ImageInfo.h"
#include <kapplication.h>
#include <kconfig.h>
#include "Settings.moc"
#include "DB/MemberMap.h"
#include <kdebug.h>

#define STR(x) QString::fromLatin1(x)

using namespace Settings;

Settings::Settings* Settings::Settings::_instance = 0;

Settings::Settings* Settings::Settings::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

Settings::Settings::Settings( const QString& imageDirectory )
    : _hasAskedAboutTimeStamps( false ), _imageDirectory( imageDirectory )
{
}

bool Settings::Settings::trustTimeStamps()
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
void Settings::Settings::setTTimeStamps( TimeStampTrust t )
{
    setValue( STR("General"), STR("trustTimeStamps"), (int) t );
}
Settings::TimeStampTrust Settings::Settings::tTimeStamps() const
{
    return (TimeStampTrust) value(  STR("General"), STR("trustTimeStamps"), (int) Always );
}

QString Settings::Settings::imageDirectory() const
{
    if ( !_imageDirectory.endsWith( STR( "/" ) ) )
        return _imageDirectory + STR( "/" );
    else
        return _imageDirectory;
}


Settings::Position Settings::Settings::infoBoxPosition() const
{
    return (Position) value( STR("Viewer"), STR("infoBoxPosition"), 0 );
}

void Settings::Settings::setInfoBoxPosition( Position pos )
{
    setValue( STR("Viewer"), STR("infoBoxPosition"), (int) pos );
}

QString Settings::Settings::HTMLBaseDir() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), QString::fromLocal8Bit(getenv("HOME")) + STR( "/public_html") );
}

void Settings::Settings::setHTMLBaseDir( const QString& dir )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), dir );
}

QString Settings::Settings::HTMLBaseURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::Settings::setHTMLBaseURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"), url );
}

QString Settings::Settings::HTMLDestURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("destUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::Settings::setHTMLDestURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("destUrl"), url );
}


void Settings::Settings::setup( const QString& imageDirectory )
{
    _instance = new Settings( imageDirectory );
}

void Settings::Settings::setCurrentLock( const ImageSearchInfo& info, bool exclude )
{
    info.saveLock();
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), exclude );
}

ImageSearchInfo Settings::Settings::currentLock() const
{
    return ImageSearchInfo::loadLock();
}

void Settings::Settings::setLocked( bool lock )
{
    bool changed = ( lock != isLocked() );
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("locked"), lock );
    if (changed)
        emit locked( lock, lockExcludes() );
}

bool Settings::Settings::isLocked() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("locked"), false );
}

bool Settings::Settings::lockExcludes() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), false );
}

void Settings::Settings::setPassword( const QString& passwd )
{
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("password"), passwd );
}

QString Settings::Settings::password() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("password"), STR("") );
}

// PENDING(blackie) move this function to Category
QString Settings::Settings::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + STR("CategoryImages" );
    member.replace( ' ', '_' );
    QString fileName = dir + STR("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}

// PENDING(blackie) move this function to Category
void Settings::Settings::setCategoryImage( const QString& category, QString member, const QImage& image )
{
    QString dir = imageDirectory() + STR("CategoryImages" );
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

// PENDING(blackie) moved this function to Category
QImage Settings::Settings::categoryImage( const QString& category, QString member, int size ) const
{
    QString fileName = fileForCategoryImage( category, member );
    QImage img;
    bool ok = img.load( fileName, "JPEG" );
    if ( ! ok ) {
        if ( ImageDB::instance()->memberMap().isGroup( category, member ) )
            img = KGlobal::iconLoader()->loadIcon( STR( "kuser" ), KIcon::Desktop, size );
        else
            img = ImageDB::instance()->categoryCollection()->categoryForName( category )->icon( size );
    }
    return img.smoothScale( size, size, QImage::ScaleMin );
}

void Settings::Settings::setViewSortType( ViewSortType tp )
{
    bool changed = ( viewSortType() != tp );
    setValue( STR("General"), STR("viewSortType"), (int) tp );
    if ( changed )
        emit viewSortTypeChanged( tp );
}

ViewSortType Settings::Settings::viewSortType() const
{
    return (ViewSortType) value( STR("General"), STR("viewSortType"), 0 );
}

void Settings::Settings::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("fromDate"), date.toString( Qt::ISODate ) );
}

QDate Settings::Settings::fromDate() const
{
    QString date = value( STR("Miscellaneous"), STR("fromDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year(), 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

void  Settings::Settings::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("toDate"), date.toString( Qt::ISODate ) );
}

QDate Settings::Settings::toDate() const
{
    QString date = value( STR("Miscellaneous"), STR("toDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year()+1, 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

QString Settings::Settings::albumCategory() const
{
    QString category = value( STR("General"), STR("albumCategory"), STR("") );

    if ( !ImageDB::instance()->categoryCollection()->categoryNames().contains( category ) ) {
        category = ImageDB::instance()->categoryCollection()->categoryNames()[0];
        const_cast<Settings*>(this)->setAlbumCategory( category );
    }

    return category;
}

void Settings::Settings::setAlbumCategory( const QString& category )
{
    setValue( STR("General"), STR("albumCategory"), category );
}

void Settings::Settings::setWindowGeometry( WindowType win, const QRect& geometry )
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    config->writeEntry( windowTypeToString( win ), geometry );
}

QRect Settings::Settings::windowGeometry( WindowType win ) const
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    QRect rect( 0,0, 800, 600 );
    return config->readRectEntry( windowTypeToString( win ), &rect );
}

bool Settings::Settings::ready()
{
    return _instance != 0;
}

int Settings::Settings::value( const QString& group, const QString& option, int defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readNumEntry( option, defaultValue );
}

QString Settings::Settings::value( const QString& group, const QString& option, const QString& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readEntry( option, defaultValue );
}

bool Settings::Settings::value( const QString& group, const QString& option, bool defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readBoolEntry( option, defaultValue );
}

QColor Settings::Settings::value( const QString& group, const QString& option, const QColor& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readColorEntry( option, &defaultValue );
}

QSize Settings::Settings::value( const QString& group, const QString& option, const QSize& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readSizeEntry( option, &defaultValue );
}

Set<QString> Settings::Settings::value(const QString& group, const QString& option, const Set<QString>& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readListEntry( option.latin1(), QStringList( defaultValue.toList() ) );
}

void Settings::Settings::setValue( const QString& group, const QString& option, int value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::Settings::setValue( const QString& group, const QString& option, const QString& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::Settings::setValue( const QString& group, const QString& option, bool value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::Settings::setValue( const QString& group, const QString& option, const QColor& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::Settings::setValue( const QString& group, const QString& option, const QSize& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::Settings::setValue( const QString& group, const QString& option, const Set<QString>& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value.toList() );
}

QSize Settings::Settings::histogramSize() const
{
    return value( STR("General"), STR("histogramSize"), QSize( 15, 30 ) );
}

void Settings::Settings::setHistogramSize( const QSize& size )
{
    bool changed = (size != histogramSize() );
    setValue( STR("General"), STR("histogramSize"), size );
    if (changed)
        emit histogramSizeChanged( size );
}

QString Settings::Settings::windowTypeToString( WindowType tp ) const
{
    switch (tp) {
    case MainWindow: return STR("MainWindow");
    case ConfigWindow: return STR("ConfigWindow");
    }
    return STR("");
}

QString Settings::Settings::groupForDatabase( const QString& setting ) const
{
    return STR("%1 - %2").arg( setting ).arg( imageDirectory() );
}



#undef STR
