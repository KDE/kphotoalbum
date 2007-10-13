/* Copyright (C) 2003-2007 Jesper K. Pedersen <blackie@kde.org>, Jan Kundrát <jkt@gentoo.org>

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
#include "FileInfo.h"
#include <qdatetime.h>
#include <qfileinfo.h>
#include "Utilities/Util.h"
#include "Settings/SettingsData.h"
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif
#include "Exif/Syncable.h"
#include <kfilemetainfo.h>
#include <kdebug.h>
#include "DB/ImageDB.h"

using namespace DB;

FileInfo FileInfo::read( const QString& fileName )
{
    return FileInfo( fileName );
}

DB::FileInfo::FileInfo( const QString& fileName )
    :_angle(0)
{
#ifdef HASEXIV2
    parseEXIV2( fileName );
#else
    parseKFileMetaInfo( fileName );
#endif
}


/**
 * Check whether the string passed as argument contains "real usable data".
 *
 * Examples of "unreal or unusable data" are strings exclusively consisting of
 * spaces only, empty strings,...
 * */
bool DB::hasRealInformation( QString string )
{
    static StringSet blacklist;
    if ( blacklist.empty() ) {
        blacklist.insert( QString::fromAscii("MINOLTA DIGITAL CAMERA") );
    }

    string = string.stripWhiteSpace();

    if ( string.isEmpty() || blacklist.count( string ) )
        return false;
    else
        return true;
}

#ifdef HASEXIV2
void DB::FileInfo::parseEXIV2( const QString& fileName )
{
    QMap<Exif::Syncable::Kind,QString> _fieldName, _visibleName;
    QMap<Exif::Syncable::Kind,Exif::Syncable::Header> _header;
    Exif::Syncable::fillTranslationTables( _fieldName, _visibleName, _header);
    Exif::Metadata metadata = Exif::Info::instance()->metadata( fileName );
    QValueList<Exif::Syncable::Kind> items;

    try { // Orientation
        items = Settings::SettingsData::instance()->orientationSyncing( false );
        for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
            bool found = false;
            switch ( *it ) {
                case Exif::Syncable::EXIF_ORIENTATION:
                {
                    Exiv2::ExifData::const_iterator field = metadata.exif.findKey( Exiv2::ExifKey( std::string( _fieldName[ *it ].ascii() ) ) );
                    if ( field != metadata.exif.end() ) {
                        int orientation =  (*field).toLong();
                        _angle = orientationToAngle( orientation );
                        found = true;
                    }
                    break;
                }
                default:
                    kdDebug() << "Unknown orientation field " << _fieldName[ *it ] << endl;
            }
            // well, it's purely hypotetical now, as we have only one possible field
            // for storing image orientation, but it's good to be ready :)
            if (found)
                break;
        }
    }
    catch( Exiv2::AnyError& e ) {
        std::ostringstream out;
        out << e;
        kdDebug() << "Exiv2 exception when parsing file " << fileName << " for orientation: " << out.str().data() << endl;
    }

    // FIXME: proper character encoding

    try { // Label
        items = Settings::SettingsData::instance()->labelSyncing( false );
        for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
            switch ( _header[ *it ] ) {
                case Exif::Syncable::EXIF:
                {
                    Exiv2::ExifData::const_iterator field = metadata.exif.findKey( Exiv2::ExifKey( _fieldName[ *it ].ascii() ) );
                    if ( field != metadata.exif.end() )
                        _label = Utilities::cStringWithEncoding( (*field).toString().c_str(),
                                Settings::SettingsData::instance()->iptcCharset() );
                    break;
                }
                case Exif::Syncable::IPTC:
                {
                    Exiv2::IptcData::const_iterator field = metadata.iptc.findKey( Exiv2::IptcKey( _fieldName[ *it ].ascii() ) );
                    if ( field != metadata.iptc.end() )
                        _label = Utilities::cStringWithEncoding( (*field).toString().c_str(),
                                Settings::SettingsData::instance()->iptcCharset() );
                    break;
                }
                case Exif::Syncable::JPEG:
                    if ( *it == Exif::Syncable::JPEG_COMMENT )
                        _label = Utilities::cStringWithEncoding( metadata.comment.c_str(), Settings::SettingsData::instance()->iptcCharset() );
                    else
                        kdDebug() << "Can't read JPEG value " << _fieldName[ *it ] << " (not implemented yet)" << endl;
                    break;
                case Exif::Syncable::FILE:
                    switch (*it) {
                        case Exif::Syncable::FILE_NAME:
                            _label = QFileInfo( fileName ).baseName( true );
                            break;
                        default:
                            kdDebug() << "Unknown field for label syncing: " << _fieldName[ *it ] << endl;
                    }
                    break;
                default:
                    kdDebug() << "Unknown label field " << _fieldName[ *it ] << endl;
            }
            if ( hasRealInformation( _label ) )
                // we have a match, let's move along
                break;
        }
    }
    catch( Exiv2::AnyError& e ) {
        std::ostringstream out;
        out << e;
        kdDebug() << "Exiv2 exception when parsing file " << fileName << " for label: " << out.str().data() << endl;
    }

    if ( !hasRealInformation( _label ) )
        _label = QFileInfo( fileName ).baseName( true );


    try { // Description
        items = Settings::SettingsData::instance()->descriptionSyncing( false );
        for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
            switch ( _header[ *it ] ) {
                case Exif::Syncable::EXIF:
                {
                    Exiv2::ExifData::const_iterator field = metadata.exif.findKey( Exiv2::ExifKey( _fieldName[ *it ].ascii() ) );
                    if ( field != metadata.exif.end() )
                        _description = Utilities::cStringWithEncoding( (*field).toString().c_str(),
                                Settings::SettingsData::instance()->iptcCharset() );
                    break;
                }
                case Exif::Syncable::IPTC:
                {
                    Exiv2::IptcData::const_iterator field = metadata.iptc.findKey( Exiv2::IptcKey( _fieldName[ *it ].ascii() ) );
                    if ( field != metadata.iptc.end() )
                        _description = Utilities::cStringWithEncoding( (*field).toString().c_str(),
                                Settings::SettingsData::instance()->iptcCharset() );
                    break;
                }
                case Exif::Syncable::JPEG:
                    if ( *it == Exif::Syncable::JPEG_COMMENT )
                        _description = Utilities::cStringWithEncoding( metadata.comment.c_str(), Settings::SettingsData::instance()->iptcCharset() );
                    else
                        kdDebug() << "Can't read JPEG value " << _fieldName[ *it ] << " (not implemented yet)" << endl;
                    break;
                default:
                    kdDebug() << "Unknown description field " << _fieldName[ *it ] << endl;
            }
            if ( hasRealInformation( _description ) )
                break;
        }
    }
    catch( Exiv2::AnyError& e ) {
        std::ostringstream out;
        out << e;
        kdDebug() << "Exiv2 exception when parsing file " << fileName << " for description: " << out.str().data() << endl;
    }

    try { // Date
        items = Settings::SettingsData::instance()->dateSyncing( false );
        for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
            switch ( _header[ *it ] ) {
                case Exif::Syncable::EXIF:
                {
                    Exiv2::ExifData::const_iterator field = metadata.exif.findKey( Exiv2::ExifKey( _fieldName[ *it ].ascii() ) );
                    if ( field != metadata.exif.end() )
                        _date = QDateTime::fromString( QString::fromLatin1( (*field).toString().c_str() ), Qt::ISODate );
                    break;
                }
                case Exif::Syncable::IPTC:
                {
                    Exiv2::IptcData::const_iterator field = metadata.iptc.findKey( Exiv2::IptcKey( _fieldName[ *it ].ascii() ) );
                    if ( field != metadata.iptc.end() )
                        _date = QDateTime::fromString( QString::fromLatin1( (*field).toString().c_str() ), Qt::ISODate );
                    break;
                }
                case Exif::Syncable::FILE:
                {
                    QFileInfo fi( fileName );
                    switch (*it) {
                        case Exif::Syncable::FILE_CTIME:
                            _date = fi.created();
                            break;
                        case Exif::Syncable::FILE_MTIME:
                            _date = fi.lastModified();
                            break;
                        default:
                            kdDebug() << "Unknown file field for date syncing: " << _fieldName[ *it ] << endl;
                    }
                    break;
                }
                default:
                    kdDebug() << "Unknown date field " << _fieldName[ *it ] << endl;
            }
            if ( _date.isValid() )
                break;
        }
    }
    catch( Exiv2::AnyError& e ) {
        std::ostringstream out;
        out << e;
        kdDebug() << "Exiv2 exception when parsing file " << fileName << " for date: " << out.str().data() << endl;
    }

    try { // Categories
        QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
        for( QValueList<DB::CategoryPtr>::iterator category = categories.begin();
                category != categories.end(); ++category )
            if ( !(*category)->isSpecialCategory() ) {
                items = Settings::SettingsData::instance()->categorySyncingFields( false, (*category)->name() );
                for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin();
                        ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {

                    QStringList stringData; // raw string data read from file
                    switch ( _header[ *it ] ) {
                        case Exif::Syncable::EXIF:
                            {
                                Exiv2::ExifData::const_iterator field = metadata.exif.findKey( Exiv2::ExifKey( _fieldName[ *it ].ascii() ) );
                                while ( field != metadata.exif.end() ) {
                                    stringData << Utilities::cStringWithEncoding( (*field).toString().c_str(),
                                            Settings::SettingsData::instance()->iptcCharset() );
                                    ++field;
                                    // FIXME: This if ugly. Either convert to
                                    // STL algorithms or bug exiv2 for providing no
                                    // find() function that takes affset to begin
                                    // search at...
                                    // The sole purpose of this is to simulate the
                                    // standard find() function for finding next key
                                    // with this value.
                                    while ( ( field != metadata.exif.end() ) && ( (*field).key() != (*field).toString().c_str() ) )
                                        ++field;
                                }
                                break;
                            }
                        case Exif::Syncable::IPTC:
                            {
                                Exiv2::IptcData::const_iterator field = metadata.iptc.findKey( Exiv2::IptcKey( _fieldName[ *it ].ascii() ) );
                                while ( field != metadata.iptc.end() ) {
                                    stringData << Utilities::cStringWithEncoding( (*field).toString().c_str(),
                                            Settings::SettingsData::instance()->iptcCharset() );
                                    ++field;
                                    // FIXME: This if ugly. Either convert to
                                    // STL algorithms or bug exiv2 for providing no
                                    // find() function that takes affset to begin
                                    // search at...
                                    // The sole purpose of this is to simulate the
                                    // standard find() function for finding next key
                                    // with this value.
                                    while ( ( field != metadata.iptc.end() ) && ( (*field).key() != (*field).toString().c_str() ) )
                                        ++field;
                                }
                                break;
                            }
                        default:
                            kdDebug() << "Reading category information from " << _fieldName[ *it ] << " is not supported" << endl; 
                    }

                    /* translate collapsed items ("Europe,Prague") into a list ("Europe", "Prague") */
                    QString separator;
                    switch ( Settings::SettingsData::instance()->categorySyncingMultiValue( (*category)->name() ) ) {
                            case Exif::Syncable::SeparateComma:
                                {
                                    separator = QString::fromAscii(",");
                                    QStringList res;
                                    for (QStringList::const_iterator it = stringData.begin(); it != stringData.end(); ++it)
                                        res += QStringList::split( separator, *it );
                                    stringData = res;
                                    break;
                                }
                            case Exif::Syncable::SeparateSemicolon:
                                {
                                    separator = QString::fromAscii(";");
                                    QStringList res;
                                    for (QStringList::const_iterator it = stringData.begin(); it != stringData.end(); ++it)
                                        res += QStringList::split( separator, *it );
                                    stringData = res;
                                    break;
                                }
                            case Exif::Syncable::Repeat:
                                // don't convert
                                break;
                    }

                    /* strip whitespace */
                    for (QStringList::iterator it = stringData.begin(); it != stringData.end(); ++it)
                        *it = (*it).stripWhiteSpace();

                    /* strip leading category identification */
                    if ( Settings::SettingsData::instance()->categorySyncingAddName( (*category)->name() ) ) {
                        const QString prefix = (*category)->name() + QString::fromAscii(":");
                        QStringList res;
                        for (QStringList::const_iterator it = stringData.begin(); it != stringData.end(); ++it) {
                            if ( (*it).startsWith( prefix ) )
                                res << (*it).mid( prefix.length() );
                            else
                                res << *it;
                        }
                        stringData = res;

                        /* strip whitespace again */
                        for (QStringList::iterator it = stringData.begin(); it != stringData.end(); ++it)
                            *it = (*it).stripWhiteSpace();

                    }

                    /* now build the category list */
                    // FIXME: this should be smart enough *not* to set
                    // Europe when input is "Europe/Prague"
                    for (QStringList::const_iterator tag = stringData.begin(); tag != stringData.end(); ++tag ) {
                        if ( (*category)->items().findIndex( *tag ) == -1 )
                            (*category)->addItem( *tag );
                        _categories[ (*category)->name() ].append( *tag );
                    }
                }
            }

    }
    catch( Exiv2::AnyError& e ) {
        std::ostringstream out;
        out << e;
        kdDebug() << "Exiv2 exception when parsing file " << fileName << " for categories: " << out.str().data() << endl;
    }
}
#endif

void DB::FileInfo::parseKFileMetaInfo( const QString& fileName )
{
    QString tempFileName( fileName );
#ifdef TEMPORARILY_REMOVED
    if ( Util::isCRW( fileName ) ) {
      QString dirName = QFileInfo( fileName ).dirPath();
      QString baseName = QFileInfo( fileName ).baseName();
      tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".thm" );
      QFileInfo tempFile (tempFileName);
      if ( !tempFile.exists() )
          tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".THM" );
    }
#endif

    KFileMetaInfo metainfo( tempFileName );
    if ( metainfo.isEmpty() )
        return;

    // Date.
    if ( metainfo.contains( QString::fromLatin1( "CreationDate" ) ) ) {
        QDate date = metainfo.value( QString::fromLatin1( "CreationDate" )).toDate();
        if ( date.isValid() ) {
            _date.setDate( date );

            if ( metainfo.contains( QString::fromLatin1( "CreationTime" ) ) ) {
                QTime time = metainfo.value(QString::fromLatin1( "CreationTime" )).toTime();
                if ( time.isValid() )
                    _date.setTime( time );
            }
        }
    }

    // Angle
    if ( metainfo.contains( QString::fromLatin1( "Orientation" ) ) )
        _angle = orientationToAngle( metainfo.value( QString::fromLatin1( "Orientation" ) ).toInt() );

    // Description
    if ( metainfo.contains( QString::fromLatin1( "Comment" ) ) )
        _description = metainfo.value( QString::fromLatin1( "Comment" ) ).toString();
}

int DB::FileInfo::orientationToAngle( int orientation )
{
    // FIXME: this needs to be revisited, some of those values actually specify
    // both rotation and flip
    if ( orientation == 1 || orientation == 2 )
        return 0;
    else if ( orientation == 3 || orientation == 4 )
        return 180;
    else if ( orientation == 5 || orientation == 8 )
        return 270;
    else if ( orientation == 6 || orientation == 7 )
        return 90;

    return 0;
}
