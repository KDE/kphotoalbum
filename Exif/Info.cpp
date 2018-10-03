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
#include "Info.h"
#include "Logging.h"

#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "Settings/SettingsData.h"
#include "Utilities/StringSet.h"
#include "Utilities/Util.h"

#include <QFileInfo>
#include <QFile>

#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>

using namespace Exif;

Info* Info::s_instance = nullptr;

QMap<QString, QStringList> Info::info( const DB::FileName& fileName, StringSet wantedKeys, bool returnFullExifName, const QString& charset )
{
    QMap<QString, QStringList> result;

    try {
        Metadata data = metadata( exifInfoFile(fileName) );

        for (Exiv2::ExifData::const_iterator i = data.exif.begin(); i != data.exif.end(); ++i) {
            QString key = QString::fromLocal8Bit(i->key().c_str());
            m_keys.insert( key );

            if ( wantedKeys.contains( key ) ) {
                QString text = key;
                if ( !returnFullExifName )
                    text = key.split(QLatin1String(".")).last();

                std::ostringstream stream;
                stream << *i;
                QString str( Utilities::cStringWithEncoding( stream.str().c_str(), charset ) );
                result[ text ] += str;
            }
        }

        for (Exiv2::IptcData::const_iterator i = data.iptc.begin(); i != data.iptc.end(); ++i) {
            QString key = QString::fromLatin1(i->key().c_str());
            m_keys.insert( key );

            if ( wantedKeys.contains( key ) ) {
                QString text = key;
                if ( !returnFullExifName )
                    text = key.split( QString::fromLatin1(".") ).last();

                std::ostringstream stream;
                stream << *i;
                QString str( Utilities::cStringWithEncoding( stream.str().c_str(), charset ) );
                result[ text ] += str;
            }
        }
    }
    catch ( ... ) {
    }

    return result;
}

Info* Info::instance()
{
    if ( !s_instance )
        s_instance = new Info;
    return s_instance;
}

StringSet Info::availableKeys()
{
    return m_keys;
}

QMap<QString, QStringList> Info::infoForViewer( const DB::FileName& fileName, const QString& charset )
{
    return info( fileName, ::Settings::SettingsData::instance()->exifForViewer(), false, charset );
}

QMap<QString, QStringList> Info::infoForDialog( const DB::FileName& fileName, const QString& charset )
{
    return info( fileName, ::Settings::SettingsData::instance()->exifForDialog(), true, charset );
}

StringSet Info::standardKeys()
{
    static StringSet res;

    if ( !res.empty() )
        return res;

    QList<const Exiv2::TagInfo*> tags;
    std::ostringstream s;

#if (EXIV2_TEST_VERSION(0,21,0))
    const Exiv2::GroupInfo* gi = Exiv2::ExifTags::groupList();
    while (gi->tagList_ != 0) {
        Exiv2::TagListFct tl     = gi->tagList_;
        const Exiv2::TagInfo* ti = tl();

        while (ti->tag_ != 0xFFFF) {
            tags << ti;
            ++ti;
        }
        ++gi;
    }

    for (QList<const Exiv2::TagInfo*>::iterator it = tags.begin(); it != tags.end(); ++it) {
        while ( (*it)->tag_ != 0xffff ) {
            res.insert(QString::fromLatin1(Exiv2::ExifKey(**it).key().c_str()));
            ++(*it);
        }
    }
#else
    tags <<
        Exiv2::ExifTags::ifdTagList() <<
        Exiv2::ExifTags::exifTagList() <<
        Exiv2::ExifTags::iopTagList() <<
        Exiv2::ExifTags::gpsTagList();
    for (QList<const Exiv2::TagInfo*>::iterator it = tags.begin(); it != tags.end(); ++it ) {
        while ( (*it)->tag_ != 0xffff ) {
            res.insert( QLatin1String(Exiv2::ExifKey( (*it)->tag_, Exiv2::ExifTags::ifdItem( (*it)->ifdId_ ) ).key().c_str() ));
            ++(*it);
        }
    }

    // Now the ugly part -- exiv2 doesn't have any way to get a list of
    // MakerNote tags in a reasonable form, so we have to parse it from strings

    for ( Exiv2::IfdId kind = Exiv2::canonIfdId; kind < Exiv2::lastIfdId;
            kind = static_cast<Exiv2::IfdId>( kind + 1 ) ) {
#if EXIV2_TEST_VERSION(0,17,0)
        Exiv2::ExifTags::taglist( s, kind );
#else
        Exiv2::ExifTags::makerTaglist( s, kind );
#endif
    }
#endif

    // IPTC tags use yet another format...
    Exiv2::IptcDataSets::dataSetList( s );

    QStringList lines = QString( QLatin1String(s.str().c_str()) ).split( QChar::fromLatin1('\n') );
    for ( QStringList::const_iterator it = lines.constBegin(); it != lines.constEnd(); ++it ) {
        if ( it->isEmpty() )
            continue;
        QStringList fields = it->split( QChar::fromLatin1('\t') );
        if ( fields.size() == 7 ) {
            QString id = fields[4];
            if ( id.endsWith( QChar::fromLatin1(',') ) )
                id.chop(1);
            res.insert( id );
        } else {
            fields = it->split( QLatin1String(", ") );
            if ( fields.size () >= 11 ) {
                res.insert( fields[8] );
            } else {
                qCWarning(ExifLog) << "Unparsable output from exiv2 library: " << *it;
                continue;
            }
        }
    }
    return res;
}

Info::Info()
{
    m_keys = standardKeys();
}

void Exif::Info::writeInfoToFile( const DB::FileName& srcName, const QString& destName )
{
    // Load Exif from source image
    Exiv2::Image::AutoPtr image =
        Exiv2::ImageFactory::open( QFile::encodeName(srcName.absolute()).data() );
    image->readMetadata();
    Exiv2::ExifData data = image->exifData();

    // Modify Exif information from database.
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( srcName );
    data["Exif.Image.ImageDescription"] = info->description().toLocal8Bit().data();

    image = Exiv2::ImageFactory::open( QFile::encodeName(destName).data() );
    image->setExifData(data);
    image->writeMetadata();
}

/**
 * Some Canon cameras stores Exif info in files ending in .thm, so we need to use those files for fetching Exif info
 * if they exists.
 */
DB::FileName Exif::Info::exifInfoFile( const DB::FileName& fileName )
{
    QString dirName = QFileInfo( fileName.relative() ).path();
    QString baseName = QFileInfo( fileName.relative() ).baseName();
    DB::FileName name = DB::FileName::fromRelativePath(dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".thm" ));
    if ( name.exists() )
        return name;

    name = DB::FileName::fromRelativePath(dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".THM" ));
    if ( name.exists() )
        return name;

    return fileName;
}

Exif::Metadata Exif::Info::metadata( const DB::FileName& fileName )
{
    try {
        Exif::Metadata result;
        Exiv2::Image::AutoPtr image =
                Exiv2::ImageFactory::open( QFile::encodeName(fileName.absolute()).data() );
        Q_ASSERT(image.get() != 0);
        image->readMetadata();
        result.exif = image->exifData();
        result.iptc = image->iptcData();
        result.comment = image->comment();
        return result;
    }
    catch ( ... ) {
    }
    return Exif::Metadata();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
