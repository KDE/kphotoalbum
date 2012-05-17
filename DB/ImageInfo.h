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

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include "GpsCoordinates.h"
#include "ImageDate.h"
#include "Utilities/Set.h"
#include "MD5.h"
#include "ExifMode.h"
#include "DB/CategoryPtr.h"
#include <QSize>
#include "FileName.h"

namespace Plugins
{
     class ImageInfo;
}

namespace XMLDB {
class Database;
}

namespace DB
{
enum PathType {
    RelativeToImageRoot,
    AbsolutePath
};

using Utilities::StringSet;
class MemberMap;

enum MediaType { Image = 0x01, Video = 0x02 };
const MediaType anyMediaType = MediaType(Image | Video);
typedef unsigned int StackID;

class ImageInfo :public KShared {

public:
    ImageInfo();
    explicit ImageInfo( const DB::FileName& fileName, MediaType type = Image, bool readExifInfo = true );
    ImageInfo( const DB::FileName& fileName,
               const QString& label,
               const QString& description,
               const ImageDate& date,
               int angle,
               const MD5& md5sum,
               const QSize& size,
               MediaType type,
               short rating = -1,
               StackID stackId = 0,
               unsigned int stackOrder = 0,
               const GpsCoordinates& geoPosition=GpsCoordinates() );
    virtual ~ImageInfo() { saveChanges(); }

    // TODO: this should have a method to access the ID.

    FileName fileName() const;
    void setFileName( const DB::FileName& relativeFileName );

    void setLabel( const QString& );
    QString label() const;

    void setDescription( const QString& );
    QString description() const;

    void setDate( const ImageDate& );
    ImageDate date() const;
    ImageDate& date();
    void readExif(const DB::FileName& fullPath, DB::ExifMode mode);

    void rotate( int degrees );
    int angle() const;
    void setAngle( int angle );

    short rating() const;
    void setRating( short rating );

    bool isStacked() const { return _stackId != 0; }
    StackID stackId() const;

    unsigned int stackOrder() const;
    void setStackOrder( const unsigned int stackOrder );

    const GpsCoordinates& geoPosition() const;
    void setGeoPosition(const GpsCoordinates& geoPosition);

    void setVideoLength(int seconds);
    int videoLength() const;

    void setCategoryInfo( const QString& key,  const StringSet& value );
    void addCategoryInfo( const QString& category, const StringSet& values );
    void addCategoryInfo( const QString& category, const QString& value );
    void clearAllCategoryInfo();
    void removeCategoryInfo( const QString& category, const StringSet& values );
    void removeCategoryInfo( const QString& category, const QString& value );

    bool hasCategoryInfo( const QString& key,  const QString& value ) const;
    bool hasCategoryInfo( const QString& key,  const StringSet& values ) const;

    QStringList availableCategories() const;
    StringSet itemsOfCategory( const QString& category ) const;
    void renameItem( const QString& key, const QString& oldValue, const QString& newValue );
    void renameCategory( const QString& oldName, const QString& newName );

    bool operator!=( const ImageInfo& other ) const;
    bool operator==( const ImageInfo& other ) const;
    ImageInfo& operator=( const ImageInfo& other );

    static bool imageOnDisk( const DB::FileName& fileName );

    const MD5& MD5Sum() const { return _md5sum; }
    void setMD5Sum( const MD5& sum ) { if (sum != _md5sum) _dirty = true; _md5sum = sum; saveChangesIfNotDelayed(); }

    void setLocked( bool );
    bool isLocked() const;

    bool isNull() const { return _null; }
    QSize size() const;
    void setSize( const QSize& size );

    MediaType mediaType() const;
    void setMediaType( MediaType type ) { if (type != _type) _dirty = true; _type = type; saveChangesIfNotDelayed(); }
    bool isVideo() const;

    void createFolderCategoryItem( DB::CategoryPtr, DB::MemberMap& memberMap );

    void delaySavingChanges(bool b=true);

    void copyExtraData( const ImageInfo& from, bool copyAngle = true);
    void removeExtraData();

protected:
    /** Save changes to database.
     *
     * Back-ends, which need changes to be instantly in database,
     * should override this.
     */
    virtual void saveChanges() {}

    void saveChangesIfNotDelayed() { if (!_delaySaving) saveChanges(); }

    void setIsNull(bool b) { _null = b; }
    bool isDirty() const { return _dirty; }
    void setIsDirty(bool b)  { _dirty = b; }
    bool updateDateInformation( int mode ) const;

    void setStackId( const StackID stackId );
    friend class XMLDB::Database;
private:
    DB::FileName _fileName;
    QString _label;
    QString _description;
    ImageDate _date;
    QMap<QString, StringSet> _categoryInfomation;
    int _angle;
    enum OnDisk { YesOnDisk, NoNotOnDisk, Unchecked };
    mutable OnDisk _imageOnDisk;
    MD5 _md5sum;
    bool _null;
    QSize _size;
    MediaType _type;
    short _rating;
    StackID _stackId;
    unsigned int _stackOrder;
    GpsCoordinates _geoPosition;
    int _videoLength;

    // Cache information
    bool _locked;

    // Will be set to true after every change
    bool _dirty;

    bool _delaySaving;
};

}

#endif /* IMAGEINFO_H */

