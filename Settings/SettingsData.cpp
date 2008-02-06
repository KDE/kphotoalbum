/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "SettingsData.h"
#include <qdir.h>
#include "Utilities/Util.h"
#include <stdlib.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qapplication.h>
#include <qcursor.h>
#include <kiconloader.h>
#include <kglobal.h>
#include "DB/ImageDB.h"
#include <qmessagebox.h>
#include "DB/CategoryCollection.h"
#include <qdatetime.h>
#include "DB/ImageInfo.h"
#include <kapplication.h>
#include <kconfig.h>
#include "SettingsData.moc"
#include "DB/MemberMap.h"
#include <qpixmapcache.h>
#ifdef SQLDB_SUPPORT
#  include "SQLDB/ConfigFileHandler.h"
#  include "SQLDB/DatabaseAddress.h"
#  include "SQLDB/QueryErrors.h"
#endif

#define STR(x) QString::fromLatin1(x)

/**
 * smoothScale() is called from the image loading thread, therefore we need
 * to cache it this way, rather than going to KConfig.
 */
static bool _smoothScale = true;

using namespace Settings;

Settings::SettingsData* Settings::SettingsData::_instance = 0;

Settings::SettingsData* Settings::SettingsData::instance()
{
    if ( ! _instance )
        qFatal("instance called before loading a setup!");
    return _instance;
}

Settings::SettingsData::SettingsData( const QString& imageDirectory )
    : _imageDirectory( imageDirectory )
{
    QPixmapCache::setCacheLimit( thumbnailCache() * 1024 );
    _smoothScale = value( QString::fromLatin1( "Viewer" ), QString::fromLatin1( "smoothScale" ), true );
}

bool Settings::SettingsData::smoothScale()
{
    return _smoothScale;
}

void Settings::SettingsData::setSmoothScale( bool b )
{
    _smoothScale = b;
    setValue( QString::fromLatin1( "Viewer" ), QString::fromLatin1( "smoothScale" ), b );
}

void Settings::SettingsData::setThumbnailAspectRatio( Settings::ThumbnailAspectRatio aspect )
{
    setValue( STR("Thumbnails"), STR("thumbnailAspectRatio"), (int) aspect);
}

Settings::ThumbnailAspectRatio Settings::SettingsData::thumbnailAspectRatio() const
{
    return (ThumbnailAspectRatio) value(  STR("Thumbnails"), STR("thumbnailAspectRatio"), (int) Aspect_4_3);
}

void Settings::SettingsData::setViewerStandardSize(StandardViewSize v)
{
    setValue( STR("Viewer"), STR("standardViewerSize"), (int) v );
}

Settings::StandardViewSize Settings::SettingsData::viewerStandardSize() const
{
    return (StandardViewSize) value(  STR("Viewer"), STR("standardViewerSize"), (int) FullSize );
}

QString Settings::SettingsData::imageDirectory() const
{
    if ( !_imageDirectory.endsWith( STR( "/" ) ) )
        return _imageDirectory + STR( "/" );
    else
        return _imageDirectory;
}


Settings::Position Settings::SettingsData::infoBoxPosition() const
{
    return (Position) value( STR("Viewer"), STR("infoBoxPosition"), 0 );
}

void Settings::SettingsData::setInfoBoxPosition( Position pos )
{
    setValue( STR("Viewer"), STR("infoBoxPosition"), (int) pos );
}

QString Settings::SettingsData::HTMLBaseDir() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), QString::fromLocal8Bit(getenv("HOME")) + STR( "/public_html") );
}

void Settings::SettingsData::setHTMLBaseDir( const QString& dir )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseDir"), dir );
}

QString Settings::SettingsData::HTMLBaseURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::SettingsData::setHTMLBaseURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("baseUrl"), url );
}

QString Settings::SettingsData::HTMLDestURL() const
{
    return value( groupForDatabase( STR("HTML Settings") ), STR("destUrl"),  STR( "file://" ) + HTMLBaseDir() );
}

void Settings::SettingsData::setHTMLDestURL( const QString& url )
{
    setValue( groupForDatabase( STR("HTML Settings") ), STR("destUrl"), url );
}


void Settings::SettingsData::setup( const QString& imageDirectory )
{
    _instance = new SettingsData( imageDirectory );
}

void Settings::SettingsData::setCurrentLock( const DB::ImageSearchInfo& info, bool exclude )
{
    info.saveLock();
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), exclude );
}

DB::ImageSearchInfo Settings::SettingsData::currentLock() const
{
    return DB::ImageSearchInfo::loadLock();
}

void Settings::SettingsData::setLocked( bool lock, bool force )
{
    bool changed = ( lock != isLocked() );
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("locked"), lock );
    if (changed || force )
        emit locked( lock, lockExcludes() );
}

bool Settings::SettingsData::isLocked() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("locked"), false );
}

bool Settings::SettingsData::lockExcludes() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("exclude"), false );
}

void Settings::SettingsData::setPassword( const QString& passwd )
{
    setValue( groupForDatabase( STR("Privacy Settings") ), STR("password"), passwd );
}

QString Settings::SettingsData::password() const
{
    return value( groupForDatabase( STR("Privacy Settings") ), STR("password"), STR("") );
}

// PENDING(blackie) move this function to Category
QString Settings::SettingsData::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = imageDirectory() + STR("CategoryImages" );
    member.replace( ' ', '_' );
    member.replace( '/', '_' );
    QString fileName = dir + STR("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}

// PENDING(blackie) move this function to Category
void Settings::SettingsData::setCategoryImage( const QString& category, QString member, const QImage& image )
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

    // PENDING(blackie) HACK ALERT: Remove all images rather than just these resolutions.
    QString key = QString::fromLatin1( "64-%2" ).arg(fileName);
    QPixmapCache::remove( key );

    key = QString::fromLatin1( "128-%2" ).arg(fileName);
    QPixmapCache::remove( key );
}

// PENDING(blackie) moved this function to Category
QPixmap Settings::SettingsData::categoryImage( const QString& category, QString member, int size ) const
{
    QString fileName = fileForCategoryImage( category, member );
    QString key = QString::fromLatin1( "%1-%2" ).arg(size).arg(fileName);
    QPixmap res;
    if ( QPixmapCache::find( key, res ) )
        return res;

    QImage img;
    bool ok = img.load( fileName, "JPEG" );
    if ( ! ok ) {
        if ( DB::ImageDB::instance()->memberMap().isGroup( category, member ) )
            img = KGlobal::iconLoader()->loadIcon( STR( "kuser" ), KIcon::Desktop, size );
        else
            img = DB::ImageDB::instance()->categoryCollection()->categoryForName( category )->icon( size );
    }
    res = Utilities::scaleImage(img, size, size, QImage::ScaleMin);

    QPixmapCache::insert( key, res );
    return res;
}

void Settings::SettingsData::setViewSortType( ViewSortType tp )
{
    bool changed = ( viewSortType() != tp );
    setValue( STR("General"), STR("viewSortType"), (int) tp );
    if ( changed )
        emit viewSortTypeChanged( tp );
}

ViewSortType Settings::SettingsData::viewSortType() const
{
    return (ViewSortType) value( STR("General"), STR("viewSortType"), 0 );
}

void Settings::SettingsData::setFromDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("fromDate"), date.toString( Qt::ISODate ) );
}

QDate Settings::SettingsData::fromDate() const
{
    QString date = value( STR("Miscellaneous"), STR("fromDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year(), 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

void  Settings::SettingsData::setToDate( const QDate& date)
{
    if (date.isValid())
        setValue( STR("Miscellaneous"), STR("toDate"), date.toString( Qt::ISODate ) );
}

QDate Settings::SettingsData::toDate() const
{
    QString date = value( STR("Miscellaneous"), STR("toDate"), STR("") );
    if ( date.isEmpty() )
        return QDate( QDate::currentDate().year()+1, 1, 1 );
    else
        return QDate::fromString( date, ISODate );
}

QString Settings::SettingsData::albumCategory() const
{
    QString category = value( STR("General"), STR("albumCategory"), STR("") );

    if ( !DB::ImageDB::instance()->categoryCollection()->categoryNames().contains( category ) ) {
        category = DB::ImageDB::instance()->categoryCollection()->categoryNames()[0];
        const_cast<SettingsData*>(this)->setAlbumCategory( category );
    }

    return category;
}

void Settings::SettingsData::setAlbumCategory( const QString& category )
{
    setValue( STR("General"), STR("albumCategory"), category );
}

void Settings::SettingsData::setWindowGeometry( WindowType win, const QRect& geometry )
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    config->writeEntry( windowTypeToString( win ), geometry );
}

QRect Settings::SettingsData::windowGeometry( WindowType win ) const
{
    KConfig* config = kapp->config();
    config->setGroup( "Window Geometry" );
    QRect rect( 0,0, 800, 600 );
    return config->readRectEntry( windowTypeToString( win ), &rect );
}

bool Settings::SettingsData::ready()
{
    return _instance != 0;
}

int Settings::SettingsData::value( const QString& group, const QString& option, int defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readNumEntry( option, defaultValue );
}

QString Settings::SettingsData::value( const QString& group, const QString& option, const QString& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readEntry( option, defaultValue );
}

bool Settings::SettingsData::value( const QString& group, const QString& option, bool defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readBoolEntry( option, defaultValue );
}

QColor Settings::SettingsData::value( const QString& group, const QString& option, const QColor& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readColorEntry( option, &defaultValue );
}

QSize Settings::SettingsData::value( const QString& group, const QString& option, const QSize& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    return config->readSizeEntry( option, &defaultValue );
}

StringSet Settings::SettingsData::value(const QString& group, const QString& option, const StringSet& defaultValue ) const
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    if ( !config->hasKey( option.latin1() ) )
        return defaultValue;
    return config->readListEntry( option );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, int value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const QString& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, bool value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const QColor& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const QSize& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value );
}

void Settings::SettingsData::setValue( const QString& group, const QString& option, const StringSet& value )
{
    KConfig* config = kapp->config();
    config->setGroup( group );
    config->writeEntry( option, value.toList() );
}

QSize Settings::SettingsData::histogramSize() const
{
    return value( STR("General"), STR("histogramSize"), QSize( 15, 30 ) );
}

void Settings::SettingsData::setHistogramSize( const QSize& size )
{
    bool changed = (size != histogramSize() );
    setValue( STR("General"), STR("histogramSize"), size );
    if (changed)
        emit histogramSizeChanged( size );
}

QString Settings::SettingsData::windowTypeToString( WindowType tp ) const
{
    switch (tp) {
    case MainWindow: return STR("MainWindow");
    case ConfigWindow: return STR("ConfigWindow");
    }
    return STR("");
}

QString Settings::SettingsData::groupForDatabase( const QString& setting ) const
{
    return STR("%1 - %2").arg( setting ).arg( imageDirectory() );
}

void Settings::SettingsData::setCategorySyncingFields( const bool writing, const QString& category, const QValueList<Exif::Syncable::Kind>& fields )
{
    _setSyncing( writing, QString::fromAscii( "category_%1").arg( category ), fields );
}

QValueList<Exif::Syncable::Kind> Settings::SettingsData::categorySyncingFields( const bool writing, const QString& category ) const
{
    return _syncing( writing, QString::fromAscii("category_%1").arg( category ) );
}

void Settings::SettingsData::setCategorySyncingSuperGroups( const QString& category, const Exif::Syncable::SuperGroupHandling how )
{
    setValue( STR("MetadataSyncing"), STR("categorySyncingSuperGroups_%1").arg( category ), static_cast<int>( how ) );
}

Exif::Syncable::SuperGroupHandling Settings::SettingsData::categorySyncingSuperGroups( const QString& category ) const
{
    return static_cast<Exif::Syncable::SuperGroupHandling>( 
            value( STR("MetadataSyncing"), STR("categorySyncingSuperGroups_%1").arg( category ), static_cast<int>(Exif::Syncable::Independent) ) );
}

void Settings::SettingsData::setCategorySyncingMultiValue(const QString& category, const Exif::Syncable::MultiValueHandling how )
{
    setValue( STR("MetadataSyncing"), STR("categorySyncingMultiValue_%1").arg( category ), static_cast<int>( how ) );
}

Exif::Syncable::MultiValueHandling Settings::SettingsData::categorySyncingMultiValue( const QString& category ) const
{
    return static_cast<Exif::Syncable::MultiValueHandling>( 
            value( STR("MetadataSyncing"), STR("categorySyncingMultiValue_%1").arg( category ), static_cast<int>(Exif::Syncable::Repeat) ) );
}

void Settings::SettingsData::setCategorySyncingAddName( const QString& category, bool include )
{
    setValue( STR("MetadataSyncing"), STR("categorySyncingAddName_%1").arg( category ), include );
}

bool Settings::SettingsData::categorySyncingAddName( const QString& category )
{
    return value( STR("MetadataSyncing"), STR("categorySyncingAddName_%1").arg( category ), false );
}

void Settings::SettingsData::_setSyncing( const bool writing, const QString& identifier, const QValueList<Exif::Syncable::Kind>& fields )
{
    QString mode = writing ? QString::fromAscii("_W") : QString::fromAscii("_R");
    QStringList list;
    for (QValueList<Exif::Syncable::Kind>::const_iterator it = fields.begin(); it != fields.end(); ++it) {
        list << QString::number( static_cast<int>( *it ) );
    }
    setValue( STR("MetadataSyncing"), STR("syncFields_%1").arg( identifier ) + mode, list.join( QString::fromAscii(";") ) );
}

QValueList<Exif::Syncable::Kind> Settings::SettingsData::_syncing( const bool writing, const QString& identifier ) const
{
    QString mode = writing ? QString::fromAscii("_W") : QString::fromAscii("_R");
    QValueList<Exif::Syncable::Kind> list, allowedItems;
    QStringList split = QStringList::split( QString::fromAscii(";"),
            value( STR("MetadataSyncing"), STR("syncFields_%1").arg( identifier ) + mode, QString::null ) );
    allowedItems = defaultFields( writing, identifier );

    for (QStringList::const_iterator it = split.begin(); it != split.end(); ++it) {
        Exif::Syncable::Kind item = static_cast<Exif::Syncable::Kind>( (*it).toInt() );
        if ( allowedItems.contains( item ) )
            list << item;
    }

    if (list.isEmpty())
        list = allowedItems;

    return list;
}

QValueList<Exif::Syncable::Kind> Settings::SettingsData::defaultFields( const bool writing, const QString& name ) const
{
    QValueList<Exif::Syncable::Kind> r, w;

    if (name == QString::fromAscii("orientation")) {
        r << Exif::Syncable::EXIF_ORIENTATION << Exif::Syncable::STOP;
        w = r;
    } else if ( name == QString::fromAscii("label")) {
        r << Exif::Syncable::FILE_NAME <<
            Exif::Syncable::STOP << Exif::Syncable::IPTC_HEADLINE <<
            Exif::Syncable::EXIF_USER_COMMENT << Exif::Syncable::EXIF_DESCRIPTION <<
            Exif::Syncable::JPEG_COMMENT << Exif::Syncable::EXIF_XPTITLE <<
            Exif::Syncable::EXIF_XPSUBJECT << Exif::Syncable::IPTC_OBJECT_NAME <<
            Exif::Syncable::IPTC_CAPTION;
        w << Exif::Syncable::IPTC_HEADLINE <<
            Exif::Syncable::STOP << Exif::Syncable::EXIF_USER_COMMENT <<
            Exif::Syncable::EXIF_DESCRIPTION << Exif::Syncable::JPEG_COMMENT <<
            Exif::Syncable::EXIF_XPTITLE << Exif::Syncable::EXIF_XPSUBJECT <<
            Exif::Syncable::IPTC_OBJECT_NAME << Exif::Syncable::IPTC_CAPTION;
    } else if ( name == QString::fromAscii("description")) {
        r << Exif::Syncable::IPTC_CAPTION <<
            Exif::Syncable::EXIF_USER_COMMENT << Exif::Syncable::EXIF_DESCRIPTION <<
            Exif::Syncable::JPEG_COMMENT << Exif::Syncable::EXIF_XPCOMMENT <<
            Exif::Syncable::EXIF_XPSUBJECT << Exif::Syncable::IPTC_OBJECT_NAME <<
            Exif::Syncable::STOP << Exif::Syncable::IPTC_HEADLINE;
        w << Exif::Syncable::IPTC_CAPTION <<
            Exif::Syncable::STOP << Exif::Syncable::EXIF_USER_COMMENT <<
            Exif::Syncable::EXIF_DESCRIPTION << Exif::Syncable::JPEG_COMMENT <<
            Exif::Syncable::EXIF_XPCOMMENT << Exif::Syncable::EXIF_XPSUBJECT <<
            Exif::Syncable::IPTC_OBJECT_NAME << Exif::Syncable::IPTC_HEADLINE;
    } else if ( name == QString::fromAscii("date")) {
        r << Exif::Syncable::EXIF_DATETIME << Exif::Syncable::EXIF_DATETIME_ORIGINAL <<
            Exif::Syncable::EXIF_DATETIME_DIGITIZED << Exif::Syncable::FILE_MTIME <<
            Exif::Syncable::FILE_CTIME << Exif::Syncable::STOP;
        w << Exif::Syncable::EXIF_DATETIME << Exif::Syncable::STOP <<
            /*Exif::Syncable::FILE_MTIME <<*/ Exif::Syncable::EXIF_DATETIME_ORIGINAL <<
            Exif::Syncable::EXIF_DATETIME_DIGITIZED;
    } else if ( name.startsWith( QString::fromAscii("category_") ) ) {
        QString category = name.mid( QString::fromAscii("category_").length() );

        // we need at least one random category for that vocabulary thingy...
        DB::CategoryPtr someCategory = *( DB::ImageDB::instance()->categoryCollection()->categories().begin() );

        if ( ( category == QString::fromLatin1("Keywords") ) ||
                ( someCategory->standardCategories()[ QString::fromLatin1("Keywords") ] == category ) ) {
            r << Exif::Syncable::STOP << Exif::Syncable::IPTC_KEYWORDS <<
                Exif::Syncable::EXIF_XPKEYWORDS << Exif::Syncable::IPTC_SUPP_CAT;
            w << Exif::Syncable::IPTC_KEYWORDS << Exif::Syncable::STOP << 
                Exif::Syncable::EXIF_XPKEYWORDS << Exif::Syncable::IPTC_SUPP_CAT;
        } else if ( ( category == QString::fromLatin1("Places") ) ||
                ( someCategory->standardCategories()[ QString::fromLatin1("Places") ] == category ) ) {
            r << Exif::Syncable::STOP <<
                Exif::Syncable::IPTC_LOCATION_CODE << Exif::Syncable::IPTC_LOCATION_NAME <<
                Exif::Syncable::IPTC_CITY << Exif::Syncable::IPTC_SUB_LOCATION <<
                Exif::Syncable::IPTC_PROVINCE_STATE << Exif::Syncable::IPTC_COUNTRY_NAME <<
                Exif::Syncable::IPTC_COUNTRY_CODE << Exif::Syncable::IPTC_SUPP_CAT;
            w << Exif::Syncable::IPTC_LOCATION_NAME << Exif::Syncable::STOP <<
                Exif::Syncable::IPTC_LOCATION_CODE << Exif::Syncable::IPTC_CITY << Exif::Syncable::IPTC_SUB_LOCATION <<
                Exif::Syncable::IPTC_PROVINCE_STATE << Exif::Syncable::IPTC_COUNTRY_NAME <<
                Exif::Syncable::IPTC_COUNTRY_CODE << Exif::Syncable::IPTC_SUPP_CAT;
        } else {
            r << Exif::Syncable::STOP << Exif::Syncable::EXIF_DESCRIPTION <<
                Exif::Syncable::EXIF_USER_COMMENT << Exif::Syncable::EXIF_XPTITLE <<
                Exif::Syncable::EXIF_XPCOMMENT << Exif::Syncable::EXIF_XPKEYWORDS <<
                Exif::Syncable::EXIF_XPSUBJECT << Exif::Syncable::IPTC_HEADLINE <<
                Exif::Syncable::IPTC_CAPTION << Exif::Syncable::IPTC_OBJECT_NAME <<
                Exif::Syncable::IPTC_SUBJECT << Exif::Syncable::IPTC_SUPP_CAT <<
                Exif::Syncable::IPTC_KEYWORDS << Exif::Syncable::IPTC_LOCATION_CODE <<
                Exif::Syncable::IPTC_LOCATION_NAME << Exif::Syncable::IPTC_CITY <<
                Exif::Syncable::IPTC_SUB_LOCATION << Exif::Syncable::IPTC_PROVINCE_STATE <<
                Exif::Syncable::IPTC_COUNTRY_CODE << Exif::Syncable::IPTC_COUNTRY_NAME;
            w = r;
        }
    }

    return writing ? w : r;
}

void Settings::SettingsData::setLabelSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields )
{
    _setSyncing( writing, QString::fromAscii("label"), fields );
}

QValueList<Exif::Syncable::Kind> Settings::SettingsData::labelSyncing( const bool writing ) const
{
    return _syncing( writing, QString::fromAscii("label") );
}

void Settings::SettingsData::setDescriptionSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields )
{
    _setSyncing( writing, QString::fromAscii("description"), fields );
}

QValueList<Exif::Syncable::Kind> Settings::SettingsData::descriptionSyncing( const bool writing ) const
{
    return _syncing( writing, QString::fromAscii("description") );
}

void Settings::SettingsData::setOrientationSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields )
{
    _setSyncing( writing, QString::fromAscii("orientation"), fields );
}

QValueList<Exif::Syncable::Kind> Settings::SettingsData::orientationSyncing( const bool writing ) const
{
    return _syncing( writing, QString::fromAscii("orientation") );
}

void Settings::SettingsData::setDateSyncing( const bool writing, const QValueList<Exif::Syncable::Kind>& fields )
{
    _setSyncing( writing, QString::fromAscii("date"), fields );
}

QValueList<Exif::Syncable::Kind> Settings::SettingsData::dateSyncing( const bool writing ) const
{
    return _syncing( writing, QString::fromAscii("date") );
}

void Settings::SettingsData::setThumbnailCache( int value )
{
    QPixmapCache::setCacheLimit( thumbnailCache() * 1024 );
    QPixmapCache::clear();
    setValue( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbnailCache" ), value );
}
int Settings::SettingsData::thumbnailCache() const
{
    return value( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbnailCache" ), 5 );
}

void Settings::SettingsData::setThumbSize( int value )
{
    QPixmapCache::clear();
    setValue( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbSize" ), value );
}
int Settings::SettingsData::thumbSize() const
{
    return value( QString::fromLatin1( "Thumbnails" ), QString::fromLatin1( "thumbSize" ), 128 );
}

#ifdef SQLDB_SUPPORT
void Settings::SettingsData::setSQLParameters(const SQLDB::DatabaseAddress& address)
{
    KConfig* config = kapp->config();
    config->setGroup(QString::fromLatin1("SQLDB"));
    SQLDB::writeConnectionParameters(address, *config);
}

SQLDB::DatabaseAddress Settings::SettingsData::getSQLParameters() const
{
    KConfig* config = kapp->config();
    config->setGroup(QString::fromLatin1("SQLDB"));
    try {
        return SQLDB::readConnectionParameters(*config);
    }
    catch (SQLDB::DriverNotFoundError&) {}
    return SQLDB::DatabaseAddress();
}
#endif /* SQLDB_SUPPORT */

#undef STR
