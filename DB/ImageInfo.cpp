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

extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
}

#include "ImageInfo.h"
#include <qfileinfo.h>
#include <qimage.h>
#include <qdom.h>
#include "Settings/SettingsData.h"
#include "Utilities/Util.h"
#include <kdebug.h>
#include <qwmatrix.h>
#include <qvariant.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"
#include "FileInfo.h"
#include <qstringlist.h>
#include "DB/MemberMap.h"
#include <kcmdlineargs.h>
#include "XMLDB/XMLCategory.h"
#include <config.h>
#include "Exif/Database.h"

using namespace DB;

ImageInfo::ImageInfo() :_null( true ), _locked( false )
{
}

ImageInfo::ImageInfo( const QString& fileName, MediaType type )
    :  _imageOnDisk( YesOnDisk ), _null( false ), _size( -1, -1 ), _type( type ), _locked( false )
{
    QString fullPath = Settings::SettingsData::instance()->imageDirectory()+ fileName;
    QFileInfo fi( Settings::SettingsData::instance()->imageDirectory() + fileName );
    _label = fi.baseName( true );
    _angle = 0;

    setFileName( fileName);

    // Read EXIF information
    readExif(fullPath, EXIFMODE_INIT);
}

void ImageInfo::setLabel( const QString& desc )
{
    _label = desc;
}

QString ImageInfo::label() const
{
    return _label;
}

void ImageInfo::setDescription( const QString& desc )
{
    _description = desc;
}

QString ImageInfo::description() const
{
    return _description;
}


void ImageInfo::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
}

void ImageInfo::addOption( const QString& key, const QStringList& value )
{
    for( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
        if (! _options[key].contains( *it ) )
            _options[key] += *it;
    }
}

void ImageInfo::removeOption( const QString& key, const QStringList& value )
{
    for( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
        if ( _options[key].contains( *it ) )
            _options[key].remove(*it);
    }
}

bool ImageInfo::hasOption( const QString& key, const QString& value )
{
    return _options[key].contains(value);
}

QStringList ImageInfo::itemsOfCategory( const QString& key ) const
{
    return _options[key];
}

void ImageInfo::renameItem( const QString& key, const QString& oldValue, const QString& newValue )
{
    QStringList& list = _options[key];
    QStringList::Iterator it = list.find( oldValue );
    if ( it != list.end() ) {
        list.remove( it );
        list.append( newValue );
    }
}

QString ImageInfo::fileName( bool relative ) const
{
    if (relative)
        return _relativeFileName;
    else
        return _absoluteFileName;
}

void ImageInfo::setFileName( const QString& relativeFileName )
{
    _relativeFileName = relativeFileName;
    setAbsoluteFileName();
    _imageOnDisk = Unchecked;
    QString folderName = Utilities::relativeFolderName( _relativeFileName );
    _options.insert( QString::fromLatin1( "Folder") , QStringList( folderName ) );
    DB::ImageDB::instance()->categoryCollection()->categoryForName(QString::fromLatin1("Folder"))->addItem( folderName );
}


void ImageInfo::rotate( int degrees )
{
    _angle += degrees + 360;
    _angle = _angle % 360;
}

int ImageInfo::angle() const
{
    return _angle;
}

void ImageInfo::setAngle( int angle )
{
    _angle = angle;
}


void ImageInfo::setDate( const ImageDate& date )
{
    _date = date;
}

ImageDate& ImageInfo::date()
{
    return _date;
}

ImageDate ImageInfo::date() const
{
    return _date;
}

bool ImageInfo::operator!=( const ImageInfo& other )
{
    return !(*this == other);
}

bool ImageInfo::operator==( const ImageInfo& other )
{
    bool changed =
        ( _relativeFileName != other._relativeFileName ||
          _label != other._label ||
          ( !_description.isEmpty() && !other._description.isEmpty() && _description != other._description ) || // one might be isNull.
          _date != other._date ||
          _angle != other._angle);
    if ( !changed ) {
        QStringList keys = DB::ImageDB::instance()->categoryCollection()->categoryNames();
        for( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
            _options[*it].sort();
            QStringList otherList = other._options[*it];
            otherList.sort();
            changed |= _options[*it] != otherList;
        }
    }
    return !changed;
}

void ImageInfo::removeOption( const QString& key, const QString& value )
{
    _options[key].remove( value );
}

Viewer::DrawList ImageInfo::drawList() const
{
    return _drawList;
}

void ImageInfo::setDrawList( const Viewer::DrawList& list )
{
    _drawList = list;
}

void ImageInfo::renameCategory( const QString& oldName, const QString& newName )
{
    _options[newName] = _options[oldName];
    _options.erase(oldName);
}

void ImageInfo::setLocked( bool locked )
{
    _locked = locked;
}

bool ImageInfo::isLocked() const
{
    return _locked;
}

void ImageInfo::readExif(const QString& fullPath, int mode)
{
    DB::FileInfo exifInfo = DB::FileInfo::read( fullPath );

    // Date
    if ( (mode & EXIFMODE_DATE) && ( (mode & EXIFMODE_FORCE) || Settings::SettingsData::instance()->trustTimeStamps() ) )
        _date = exifInfo.dateTime();

    // Orientation
    if ( (mode & EXIFMODE_ORIENTATION) && Settings::SettingsData::instance()->useEXIFRotate() )
        _angle = exifInfo.angle();

    // Description
    if ( (mode & EXIFMODE_DESCRIPTION) && Settings::SettingsData::instance()->useEXIFComments() )
        _description = exifInfo.description();

    // Database update
    if ( mode & EXIFMODE_DATABASE_UPDATE ) {
#ifdef HASEXIV2
        Exif::Database::instance()->add( fullPath );
#endif
    }
}


QStringList ImageInfo::availableCategories() const
{
    return _options.keys();
}

void ImageInfo::clearMatched() const
{
    _matched.clear();
}

void ImageInfo::setMatched( const QString& category, const QString& value ) const
{
    _matched[category].insert( value );
    const MemberMap& map = DB::ImageDB::instance()->memberMap();
    QStringList members = map.members( category, value, true );
    _matched[category].insert( members );
}

/**
 * Returns whether all tokens for the given image are matches by the search
 * example: returns true if all people on an image is in the search,
 * i.e. it is only true if there are no persons on the image that are not
 * explicit searched for.
 * This is used when the search includes *None*
 */
bool ImageInfo::allMatched( const QString& category )
{
    const QStringList list = itemsOfCategory( category );
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        if ( !_matched[category].contains( *it ) )
            return false;
    }

    return true;
}

bool ImageInfo::imageOnDisk() const
{
    if ( _imageOnDisk == Unchecked ) {
        QFileInfo fi( fileName() );
        _imageOnDisk = (fi.exists() ? YesOnDisk : NoNotOnDisk);
    }
    return _imageOnDisk == YesOnDisk;
}

QSize ImageInfo::size() const
{
    return _size;
}

void ImageInfo::setSize( const QSize& size )
{
    _size = size;
}

bool ImageInfo::imageOnDisk( const QString& fileName )
{
    QFileInfo fi( fileName );
    return fi.exists();
}

ImageInfo::ImageInfo( const QString& fileName,
                      const QString& label,
                      const QString& description,
                      const ImageDate& date,
                      int angle,
                      const QString& md5sum,
                      const QSize& size,
                      MediaType type )
{
    _relativeFileName = fileName;
    setAbsoluteFileName();
    _label =label;
    _description =description;
    _date = date;
    _angle =angle;
    _md5sum =md5sum;
    _size = size;
    _imageOnDisk = Unchecked;
    _locked = false;
    _null = false;
    _type = type;
}

ImageInfo& ImageInfo::operator=( const ImageInfo& other )
{
    _relativeFileName = other._relativeFileName;
    setAbsoluteFileName();
    _label = other._label;
    _description = other._description;
    _date = other._date;
    _options = other._options;
    _angle = other._angle;
    _drawList = other._drawList;
    _imageOnDisk = other._imageOnDisk;
    _md5sum = other._md5sum;
    _null = other._null;
    _size = other._size;

    return *this;
}

void ImageInfo::addDrawing( const QDomElement& elm )
{
    _drawList.load( elm );
}

MediaType DB::ImageInfo::mediaType() const
{
    return _type;
}

/**
 * During profiling I found that it took almost 5% of the time during
 * categorizing when browsing, simply to calculate the absolute filename, therefore it is
 * now an instance variable rather than calculated dynamically in
 * fileName().
 */
void DB::ImageInfo::setAbsoluteFileName()
{
    _absoluteFileName = Settings::SettingsData::instance()->imageDirectory() + _relativeFileName;
}

