/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config-kpa-kipi.h>
#ifdef HASKIPI
#include "Plugins/ImageInfo.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "DB/Category.h"
#include "DB/CategoryPtr.h"
#include "DB/MemberMap.h"
#include "MainWindow/DirtyIndicator.h"
#include <QList>
#include "DB/CategoryCollection.h"
#include <QFileInfo>
#include <QDebug>

#define KEXIV_ORIENTATION_UNSPECIFIED   0
#define KEXIV_ORIENTATION_NORMAL        1
#define KEXIV_ORIENTATION_HFLIP         2
#define KEXIV_ORIENTATION_ROT_180       3
#define KEXIV_ORIENTATION_VFLIP         4
#define KEXIV_ORIENTATION_ROT_90_HFLIP  5
#define KEXIV_ORIENTATION_ROT_90        6
#define KEXIV_ORIENTATION_ROT_90_VFLIP  7
#define KEXIV_ORIENTATION_ROT_270       8
/**
 * Convert a rotation in degrees to a KExiv2::ImageOrientation value.
 */
static int deg2KexivOrientation( int deg)
{
    deg = (deg + 360) % 360;;
    switch (deg)
    {
        case 0:
            return KEXIV_ORIENTATION_NORMAL;
        case 90:
            return KEXIV_ORIENTATION_ROT_90;
        case 180:
            return KEXIV_ORIENTATION_ROT_180;
        case 270:
            return KEXIV_ORIENTATION_ROT_270;
        default:
            qWarning() << "Rotation of " << deg << "degrees can't be mapped to KExiv2::ImageOrientation value.";
            return KEXIV_ORIENTATION_UNSPECIFIED;
    }
}
/**
 * Convert a KExiv2::ImageOrientation value into a degrees angle.
 */
static int kexivOrientation2deg( int orient)
{
    switch (orient)
    {
        case KEXIV_ORIENTATION_NORMAL:
            return 0;
        case KEXIV_ORIENTATION_ROT_90:
            return 90;
        case KEXIV_ORIENTATION_ROT_180:
            return 280;
        case KEXIV_ORIENTATION_ROT_270:
            return 270;
        default:
            qWarning() << "KExiv2::ImageOrientation value " << orient << " not a pure rotation. Discarding orientation info.";
            return 0;
    }
}

Plugins::ImageInfo::ImageInfo( KIPI::Interface* interface, const KUrl& url )
    : KIPI::ImageInfoShared( interface, url )
{
    _info = DB::ImageDB::instance()->info( DB::FileName::fromAbsolutePath(_url.path()));
}

QMap<QString,QVariant> Plugins::ImageInfo::attributes()
{
    Q_ASSERT( _info );
    QMap<QString,QVariant> res;

    res.insert(QString::fromLatin1("name"), QFileInfo(_info->fileName().absolute()).baseName());
    res.insert(QString::fromLatin1("comment"), _info->description());

    res.insert(QLatin1String("date"), _info->date().start());
    res.insert(QLatin1String("dateto"), _info->date().end());
    res.insert(QLatin1String("isexactdate"), _info->date().start() == _info->date().end());

    res.insert(QString::fromLatin1("orientation"), deg2KexivOrientation(_info->angle()) );
    res.insert(QString::fromLatin1("angle"), deg2KexivOrientation(_info->angle()) ); // for compatibility with older versions. Now called orientation.

    res.insert(QString::fromLatin1("title"), _info->label());

    res.insert(QString::fromLatin1("rating"), _info->rating());

    // not supported:
    //res.insert(QString::fromLatin1("colorlabel"), xxx );
    //res.insert(QString::fromLatin1("picklabel"), xxx );

    DB::GpsCoordinates position = _info->geoPosition();
    if (!position.isNull()) {
        res.insert(QString::fromLatin1("longitude"), QVariant(position.longitude()));
        res.insert(QString::fromLatin1("latitude"), QVariant(position.latitude()));
        res.insert(QString::fromLatin1("altitude"), QVariant(position.altitude()));
        if (position.precision() != DB::GpsCoordinates::NoPrecisionData) {
            // XXX: attribute not mentioned in libkipi/imageinfo.h -> is this supported?
            res.insert(QString::fromLatin1("positionPrecision"), QVariant(position.precision()));
        }
    }

    // Flickr plug-in expects the item tags, so we better give them.
    QString text;
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    QStringList tags;
    QStringList tagspath;
    const QLatin1String sep ("/");
    for( QList<DB::CategoryPtr>::Iterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        QString categoryName = (*categoryIt)->name();
        if ( (*categoryIt)->isSpecialCategory() )
            continue;
        // I don't know why any categories except the above should be excluded
        //if ( (*categoryIt)->doShow() ) {
            Utilities::StringSet items = _info->itemsOfCategory( categoryName );
            for( Utilities::StringSet::Iterator it = items.begin(); it != items.end(); ++it ) {
                tags.append( *it );
                // digikam compatible tag path:
                // note: this produces a semi-flattened hierarchy.
                // instead of "Places/France/Paris" this will yield "Places/Paris"
                tagspath.append( categoryName + sep + (*it) );
            }
        //}
    }
    res.insert(QString::fromLatin1( "tagspath" ), tagspath );
    res.insert(QString::fromLatin1( "keywords" ), tags );
    res.insert(QString::fromLatin1( "tags" ), tags ); // for compatibility with older versions. Now called keywords.

    // TODO: implement this:
    //res.insert(QString::fromLatin1( "filesize" ), xxx );

    // not supported:
    //res.insert(QString::fromLatin1( "creators" ), xxx );
    //res.insert(QString::fromLatin1( "credit" ), xxx );
    //res.insert(QString::fromLatin1( "rights" ), xxx );
    //res.insert(QString::fromLatin1( "source" ), xxx );

    return res;
}

void Plugins::ImageInfo::clearAttributes()
{
    if( _info ) {
        // official behaviour is to delete all officially supported attributes:
        QStringList attr;
        attr.append( QString::fromLatin1("comment") );
        attr.append( QString::fromLatin1("date") );
        attr.append( QString::fromLatin1("title") );
        attr.append( QString::fromLatin1("orientation") );
        attr.append( QString::fromLatin1("tagspath") );
        attr.append( QString::fromLatin1("rating") );
        attr.append( QString::fromLatin1("colorlabel") );
        attr.append( QString::fromLatin1("picklabel") );
        attr.append( QString::fromLatin1("gpslocation") );
        attr.append( QString::fromLatin1("copyrights") );
        delAttributes(attr);
    }
}

void Plugins::ImageInfo::addAttributes( const QMap<QString,QVariant>& amap )
{
    if ( _info && ! amap.empty() ) {
        QMap<QString,QVariant> map = amap;
        if ( map.contains(QLatin1String("name")) )
        {
            // plugin renamed the item
            // TODO: implement this
            qWarning("File renaming by kipi-plugin not supported.");
            //map.remove(QLatin1String("name"));
        }
        if ( map.contains(QLatin1String("comment")) )
        {
            // is it save to do that? digikam seems to allow multiple comments on a single image
            // if a plugin assumes that it is adding a comment, not setting it, things might go badly...
            _info->setDescription( map[QLatin1String("comment")].toString() );
            map.remove(QLatin1String("comment"));
        }
        // note: this probably won't work as expected because according to the spec,
        // "isexactdate" is supposed to be readonly and therefore never set here:
        if (map.contains(QLatin1String("isexactdate")) && map.contains(QLatin1String("date"))) {
            _info->setDate(DB::ImageDate(map[QLatin1String("date")].toDateTime()));
            map.remove(QLatin1String("date"));
        } else if (map.contains(QLatin1String("date")) && map.contains(QLatin1String("dateto"))) {
            _info->setDate(DB::ImageDate(map[QLatin1String("date")].toDateTime(), map[QLatin1String("dateto")].toDateTime()));
            map.remove(QLatin1String("date"));
            map.remove(QLatin1String("dateto"));
        } else if (map.contains(QLatin1String("date"))) {
            _info->setDate(DB::ImageDate(map[QLatin1String("date")].toDateTime()));
            map.remove(QLatin1String("date"));
        }
        if ( map.contains(QLatin1String("angle")) )
        {
            qWarning("Kipi-plugin uses deprecated attribute \"angle\".");
            _info->setAngle( kexivOrientation2deg( map[QLatin1String("angle")].toInt() ) );
            map.remove(QLatin1String("angle"));
        }
        if ( map.contains(QLatin1String("orientation")) )
        {
            _info->setAngle( kexivOrientation2deg( map[QLatin1String("orientation")].toInt() ) );
            map.remove(QLatin1String("orientation"));
        }
        if ( map.contains(QLatin1String("title")) )
        {
            _info->setLabel( map[QLatin1String("title")].toString() );
            map.remove(QLatin1String("title"));
        }
        if ( map.contains(QLatin1String("rating")) )
        {
            _info->setRating( map[QLatin1String("rating")].toInt() );
            map.remove(QLatin1String("rating"));
        }
        // I don't know whether we should loosen the warning on this one:
        // after all, a Plugin might update one component of the gps data only...
        if (map.contains(QLatin1String("longitude")) ||
                map.contains(QLatin1String("latitude")) ||
                map.contains(QLatin1String("altitude")) ||
                map.contains(QLatin1String("positionPrecision"))) {
            if (map.contains(QLatin1String("longitude")) && map.contains(QLatin1String("latitude"))) {
                double altitude = 0;
                QVariant var = map[QLatin1String("altitude")];
                if (!var.isNull()) {
                    altitude = var.toDouble();
                }
                int precision = DB::GpsCoordinates::NoPrecisionData;
                var = map[QLatin1String("positionPrecision")];
                if (!var.isNull()) {
                    precision = var.toInt();
                }
                DB::GpsCoordinates coord(map[QLatin1String("longitude")].toDouble(), map[QLatin1String("latitude")].toDouble(), altitude, precision);
                _info->setGeoPosition(coord);
                map.remove(QLatin1String("longitude"));
                map.remove(QLatin1String("latitude"));
                map.remove(QLatin1String("altitude"));
                map.remove(QLatin1String("positionPrecision"));
            } else {
                qWarning("Geo coordinates incomplete. Need at least 'longitude' and 'latitude', optionally 'altitude' and 'positionPrecision'");
            }
        }
        if ( map.contains(QLatin1String("tagspath")) )
        {
            const QStringList tagspaths = map[QLatin1String("tagspath")].toStringList();
            const DB::CategoryCollection *categories = DB::ImageDB::instance()->categoryCollection();
            DB::MemberMap &memberMap = DB::ImageDB::instance()->memberMap();
            Q_FOREACH( const QString &path, tagspaths )
            {
                qDebug() << "Adding tags: " << path;
                QStringList tagpath = path.split( QLatin1String("/"), QString::SkipEmptyParts );
                // Note: maybe taggspaths with only one component or with unknown first component
                //  should be added to the "keywords"/"Events" category?
                if ( tagpath.size() < 2 )
                {
                    qWarning() << "Ignoring incompatible tag: " << path;
                    continue;
                }

                // first component is the category,
                const QString categoryName = tagpath.takeFirst();
                DB::CategoryPtr cat = categories->categoryForName( categoryName );
                if ( ! cat.isNull() )
                {
                    QString previousTag;
                    // last component is the tag:
                    // others define hierarchy:
                    Q_FOREACH( const QString &currentTag, tagpath )
                    {
                        if ( ! cat->items().contains( currentTag ) )
                        {
                            qDebug() << "Adding tag " << currentTag << " to category " << categoryName;
                            // before we can use a tag, we have to add it
                            cat->addItem( currentTag );
                        }
                        if ( ! previousTag.isNull() )
                        {
                            if ( ! memberMap.isGroup( categoryName, previousTag ) )
                            {
                                // create a group for the parent tag, so we can add a sub-category
                                memberMap.addGroup(  categoryName, previousTag );
                            }
                            if ( memberMap.canAddMemberToGroup( categoryName, previousTag, currentTag ) )
                            {
                                // make currentTag a member of the previousTag group
                                memberMap.addMemberToGroup( categoryName, previousTag, currentTag );
                            } else {
                                qWarning() << "Cannot make " << currentTag << " a subcategory of "
                                    << categoryName << "/" << previousTag << "!";
                            }
                        }
                        previousTag = currentTag;
                    }
                    qDebug() << "Adding tag " << previousTag << " in category " << categoryName
                        << " to image " << _info->label();
                    // previousTag must be a valid category (see addItem() above...)
                    _info->addCategoryInfo( categoryName, previousTag );
                } else {
                    qWarning() << "Unknown category: " << categoryName;
                }
            }
            map.remove(QLatin1String("tagspath"));
        }

        // remove read-only keywords:
        map.remove(QLatin1String("filesize"));
        map.remove(QLatin1String("isexactdate"));
        map.remove(QLatin1String("keywords"));
        map.remove(QLatin1String("tags"));

        // colorlabel
        // picklabel
        // creators
        // credit
        // rights
        // source

        MainWindow::DirtyIndicator::markDirty();
        if ( ! map.isEmpty() )
        {
            qWarning() << "The following attributes are not (yet) supported by the KPhotoAlbum KIPI interface:" << map;
        }
    }
}

void Plugins::ImageInfo::delAttributes( const QStringList& attrs)
{
    if ( _info && ! attrs.empty() ) {
        QStringList delAttrs = attrs;
        if ( delAttrs.contains(QLatin1String("comment")))
        {
            _info->setDescription( QString() );
            delAttrs.removeAll(QLatin1String("comment"));
        }
        // not supported: date
        if ( delAttrs.contains(QLatin1String("orientation")) ||
                delAttrs.contains(QLatin1String("angle")) )
        {
            _info->setAngle( 0 );
            delAttrs.removeAll(QLatin1String("orientation"));
            delAttrs.removeAll(QLatin1String("angle"));
        }
        if ( delAttrs.contains(QLatin1String("title")))
        {
            _info->setLabel( QString() );
            delAttrs.removeAll(QLatin1String("title"));
        }
        // TODO:
        // rating
        // (colorlabel)
        // (picklabel)
        // copyrights
        if ( delAttrs.contains(QLatin1String("tags")) ||
                delAttrs.contains(QLatin1String("tagspath")))
        {
            _info->clearAllCategoryInfo();
            delAttrs.removeAll(QLatin1String("tags"));
            delAttrs.removeAll(QLatin1String("tagspath"));
        }
        if ( delAttrs.contains(QLatin1String("gpslocation")) )
        {
            //clear position:
            _info->setGeoPosition(DB::GpsCoordinates());
            delAttrs.removeAll(QLatin1String("gpslocation"));
        }
        MainWindow::DirtyIndicator::markDirty();
        if ( ! delAttrs.isEmpty() )
        {
            qWarning() << "The following attributes are not (yet) supported by the KPhotoAlbum KIPI interface:" << delAttrs;
        }
    }
}

#if KIPI_VERSION >= 0x010200
#define KIPI_IMAGEINFO_CONST_MODIFIER const
#else
#define KIPI_IMAGEINFO_CONST_MODIFIER
#endif
void Plugins::ImageInfo::cloneData( ImageInfoShared* KIPI_IMAGEINFO_CONST_MODIFIER other )
{
    ImageInfoShared::cloneData( other );
    if ( _info ) {
        Plugins::ImageInfo* inf = static_cast<Plugins::ImageInfo*>( other );
        _info->setDate( inf->_info->date() );
        MainWindow::DirtyIndicator::markDirty();
    }
}
#undef KIPI_IMAGEINFO_CONST_MODIFIER


#if KIPI_VERSION < 0x010500
#if KIPI_VERSION >= 0x010300
#define KIPI_IMAGEINFO_FILINAMEFUN_GETTER_NAME name
#define KIPI_IMAGEINFO_FILINAMEFUN_SETTER_NAME setName
#else
#define KIPI_IMAGEINFO_FILINAMEFUN_GETTER_NAME title
#define KIPI_IMAGEINFO_FILINAMEFUN_SETTER_NAME setTitle
#endif
QString Plugins::ImageInfo::KIPI_IMAGEINFO_FILINAMEFUN_GETTER_NAME()
{
    if ( _info )
        return _info->label();
    else
        return QString();
}

void Plugins::ImageInfo::KIPI_IMAGEINFO_FILINAMEFUN_SETTER_NAME( const QString& name )
{
    if ( _info ) {
        _info->setLabel( name );
        MainWindow::DirtyIndicator::markDirty();
    }
}
#undef KIPI_IMAGEINFO_FILINAMEFUN_GETTER_NAME
#undef KIPI_IMAGEINFO_FILINAMEFUN_SETTER_NAME

QString Plugins::ImageInfo::description()
{
    if ( _info )
        return _info->description();
    else
        return QString();
}

void Plugins::ImageInfo::setDescription( const QString& description )
{
    if ( _info ) {
        _info->setDescription( description );
        MainWindow::DirtyIndicator::markDirty();
    }
}

int Plugins::ImageInfo::angle()
{
    if ( _info )
        return _info->angle();
    else
        return 0;
}

void Plugins::ImageInfo::setAngle( int angle )
{
    if ( _info ) {
        _info->setAngle( angle );
        MainWindow::DirtyIndicator::markDirty();
    }
}

QDateTime Plugins::ImageInfo::time( KIPI::TimeSpec what )
{
    if ( _info ) {
        if ( what == KIPI::FromInfo ) {
            return _info->date().start() ;
        }
        else
            return _info->date().end();
    }
    else
        return KIPI::ImageInfoShared::time( what );
}

bool Plugins::ImageInfo::isTimeExact()
{
    if ( !_info )
        return true;
    return _info->date().hasValidTime();
}

void Plugins::ImageInfo::setTime( const QDateTime& time, KIPI::TimeSpec spec )
{
    if ( !_info )
        return;
    if ( spec == KIPI::FromInfo ) {
        _info->setDate( DB::ImageDate( time, time ) );
        MainWindow::DirtyIndicator::markDirty();
    }
    else {
        DB::ImageDate date = _info->date();
        _info->setDate( DB::ImageDate( date.start(), time ) );
        MainWindow::DirtyIndicator::markDirty();
    }
}
#endif // KIPI_VERSION < 0x010500

bool Plugins::ImageInfo::isPositionAttribute(const QString &key)
{
    return (key == QString::fromLatin1("longitude") || key == QString::fromLatin1("latitude") || key == QString::fromLatin1("altitude") || key == QString::fromLatin1("positionPrecision"));
}

bool Plugins::ImageInfo::isCategoryAttribute(const QString &key)
{
    return (key != QString::fromLatin1("tags") && !isPositionAttribute(key));
}

#endif // KIPI
// vi:expandtab:tabstop=4 shiftwidth=4:
