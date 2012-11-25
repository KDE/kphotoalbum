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
#include "MainWindow/DirtyIndicator.h"
#include <QList>
#include "DB/CategoryCollection.h"
#include <QFileInfo>

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

    res.insert(QString::fromLatin1("orientation"), _info->angle());
    res.insert(QString::fromLatin1("angle"), _info->angle()); // for compatibility with older versions. Now called orientation.

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
    for( QList<DB::CategoryPtr>::Iterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        QString categoryName = (*categoryIt)->name();
        if ( (*categoryIt)->isSpecialCategory() )
            continue;
        if ( (*categoryIt)->doShow() ) {
            Utilities::StringSet items = _info->itemsOfCategory( categoryName );
            for( Utilities::StringSet::Iterator it = items.begin(); it != items.end(); ++it ) {
                tags.append( *it );
            }
        }
    }
    res.insert(QString::fromLatin1( "tagspath" ), tags );
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
        _info->clearAllCategoryInfo();
        // TODO I think it is reasonable to keep the gps position anyway, isn't it?
        MainWindow::DirtyIndicator::markDirty();
        // official behaviour is to delete all officially supported attributes:
        //  QStringList attr;
        //  attr.append("comment");
        //  attr.append("date");
        //  attr.append("title");
        //  attr.append("orientation");
        //  attr.append("tagspath");
        //  attr.append("rating");
        //  attr.append("colorlabel");
        //  attr.append("picklabel");
        //  attr.append("gpslocation");
        //  attr.append("copyrights");
        // delAttributes(attr);
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
            _info->setAngle( map[QLatin1String("angle")].toInt() );
            map.remove(QLatin1String("angle"));
        }
        if ( map.contains(QLatin1String("orientation")) )
        {
            _info->setAngle( map[QLatin1String("orientation")].toInt() );
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

        // remove read-only keywords:
        map.remove(QLatin1String("filesize"));
        map.remove(QLatin1String("isexactdate"));
        map.remove(QLatin1String("keywords"));
        map.remove(QLatin1String("tags"));

        // FIXME: how exactly is this supposed to work?
        // all this will do is to set the following categories to some (mostly meaningless?) value:
        // colorlabel
        // picklabel
        // creators
        // credit
        // rights
        // source
        for( QMap<QString,QVariant>::ConstIterator it = map.begin(); it != map.end(); ++it ) {
            QStringList list = it.value().toStringList();
            if (isCategoryAttribute(it.key())) {
                _info->addCategoryInfo( it.key(), list.toSet() );
            }
        }

        MainWindow::DirtyIndicator::markDirty();
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
        // --> mail miika if the "freeform tag set/delete" works with any plugin, currently
        if ( delAttrs.contains(QLatin1String("tags")) ||
                delAttrs.contains(QLatin1String("keywords")) ||
                delAttrs.contains(QLatin1String("tagspath")))
        {
            _info->clearAllCategoryInfo();
            delAttrs.removeAll(QLatin1String("tags"));
            delAttrs.removeAll(QLatin1String("keywords"));
            delAttrs.removeAll(QLatin1String("tagspath"));
        }
        if ( delAttrs.contains(QLatin1String("gpslocation")) )
        {
            //clear position:
            _info->setGeoPosition(DB::GpsCoordinates());
            delAttrs.removeAll(QLatin1String("gpslocation"));
        }
        // FIXME: how exactly is this supposed to work?
        for (QStringList::const_iterator it = delAttrs.begin(); it != delAttrs.end(); ++it) {
            if (isCategoryAttribute(*it)) {
                _info->setCategoryInfo(*it, Utilities::StringSet());
            }
        }
        MainWindow::DirtyIndicator::markDirty();
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
