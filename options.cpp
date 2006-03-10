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
#include "membermap.h"
#include <kdebug.h>

#define STR(x) QString::fromLatin1(x)

Options* Options::_instance = 0;

Options* Options::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

Options::Options( const QString& imageDirectory )
    : _hasAskedAboutTimeStamps( false ), _imageDirectory( imageDirectory )
{
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
    setValue( STR("General"), STR("trustTimeStamps"), (int) t );
}

Options::TimeStampTrust Options::tTimeStamps() const
{
    return (TimeStampTrust) value(  STR("General"), STR("trustTimeStamps"), (int) Always );
}

QString Options::imageDirectory() const
{
    if ( !_imageDirectory.endsWith( STR( "/" ) ) )
        return _imageDirectory + STR( "/" );
    else
        return _imageDirectory;
}


Options::Position Options::infoBoxPosition() const
{
    return (Position) value( STR("Viewer"), STR("infoBoxPosition"), 0 );
}

void Options::setInfoBoxPosition( Position pos )
{
    setValue( STR("Viewer"), STR("infoBoxPosition"), (int) pos );
}

QString Options::HTMLBaseDir() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), QString::fromLocal8Bit(getenv("HOME")) + STR( "/public_html") );
}

void Options::setHTMLBaseDir( const QString& dir )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), dir );
}

QString Options::HTMLBaseURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Options::setHTMLBaseURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"), url );
}

QString Options::HTMLDestURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("destUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Options::setHTMLDestURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("destUrl"), url );
}


void Options::setup( const QString& imageDirectory )
{
    _instance = new Options( imageDirectory );
}

void Options::setCurrentLock( const ImageSearchInfo& info, bool exclude )
{
    info.saveLock();
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), exclude );
}

ImageSearchInfo Options::currentLock() const
{
    return ImageSearchInfo::loadLock();
}

void Options::setLocked( bool lock )
{
    bool changed = ( lock != isLocked() );
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("locked"), lock );
    if (changed)
        emit locked( lock, lockExcludes() );
}

bool Options::isLocked() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("locked"), false );
}

bool Options::lockExcludes() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), false );
}

void Options::setPassword( const QString& passwd )
{
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("password"), passwd );
}

QString Options::password() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("password"), STR("") );
}

// PENDING(blackie) move this function to Category
QString Options::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + STR("CategoryImages" );
    member.replace( ' ', '_' );
    QString fileName = dir + STR("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}

// PENDING(blackie) move this function to Category
void Options::setCategoryImage( const QString& category, QString member, const QImage& image )
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
QImage Options::categoryImage( const QString& category, QString member, int size ) const
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

void Options::setViewSortType( ViewSortType tp )
{
    bool changed = ( viewSortType() != tp );
    setValue( STR("General"), STR("viewSortType"), (int) tp );
    if ( changed )
        emit viewSortTypeChanged( tp );
}

Options::ViewSortType Options::viewSortType() const
{
    return (ViewSortType) value( STR("General"), STR("viewSortType"), 0 );
}

void Options::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("fromDate"), date.toString( Qt::ISODate ) );
}

QDate Options::fromDate() const
{
    QString date = value( STR("Miscellaneous"), STR("fromDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year(), 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

void  Options::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("toDate"), date.toString( Qt::ISODate ) );
}

QDate Options::toDate() const
{
    QString date = value( STR("Miscellaneous"), STR("toDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year()+1, 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

QString Options::albumCategory() const
{
    QString category = value( STR("General"), STR("albumCategory"), STR("") );

    if ( !ImageDB::instance()->categoryCollection()->categoryNames().contains( category ) ) {
        category = ImageDB::instance()->categoryCollection()->categoryNames()[0];
        const_cast<Options*>(this)->setAlbumCategory( category );
    }

    return category;
}

void Options::setAlbumCategory( const QString& category )
{
    setValue( STR("General"), STR("albumCategory"), category );
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

int Options::value( const QString& group, const QString& option, int defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readNumEntry( option, defaultValue );
}

QString Options::value( const QString& group, const QString& option, const QString& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readEntry( option, defaultValue );
}

bool Options::value( const QString& group, const QString& option, bool defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readBoolEntry( option, defaultValue );
}

QColor Options::value( const QString& group, const QString& option, const QColor& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readColorEntry( option, &defaultValue );
}

QSize Options::value( const QString& group, const QString& option, const QSize& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readSizeEntry( option, &defaultValue );
}

Set<QString> Options::value(const QString& group, const QString& option, const Set<QString>& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readListEntry( option.latin1(), QStringList( defaultValue.toList() ) );
}

void Options::setValue( const QString& group, const QString& option, int value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const QString& group, const QString& option, const QString& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const QString& group, const QString& option, bool value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const QString& group, const QString& option, const QColor& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const QString& group, const QString& option, const QSize& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Options::setValue( const QString& group, const QString& option, const Set<QString>& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value.toList() );
}

QSize Options::histogramSize() const
{
    return value( STR("General"), STR("histogramSize"), QSize( 15, 30 ) );
}

void Options::setHistogramSize( const QSize& size )
{
    bool changed = (size != histogramSize() );
    setValue( STR("General"), STR("histogramSize"), size );
    if (changed)
        emit histogramSizeChanged( size );
}

QString Options::windowTypeToString( WindowType tp ) const
{
    switch (tp) {
    case MainWindow: return STR("MainWindow");
    case ConfigWindow: return STR("ConfigWindow");
    }
    return STR("");
}

QString Options::groupForDatabase( const QString& setting ) const
{
    return STR("%1 - %2").arg( setting ).arg( imageDirectory() );
}



#undef STR
