/* Copyright (C) 2003-2015 Jesper K. Pedersen <blackie@kde.org>

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
#include "ImageDate.h"
#include "Utilities/StringSet.h"
#include "MD5.h"
#include "ExifMode.h"
#include "DB/CategoryPtr.h"
#include <QSize>
#include <QRect>
#include "FileName.h"

#include "config-kpa-kgeomap.h"
#ifdef HAVE_KGEOMAP
#include <KGeoMap/GeoCoordinates>
#endif

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
enum RotationMode {
    RotateImageInfoAndAreas,
    RotateImageInfoOnly
};

using Utilities::StringSet;
class MemberMap;

enum MediaType { Image = 0x01, Video = 0x02 };
const MediaType anyMediaType = MediaType(Image | Video);
typedef unsigned int StackID;

class ImageInfo :public QSharedData {

public:
    ImageInfo();
    explicit ImageInfo( const DB::FileName& fileName, MediaType type = Image, bool readExifInfo = true, bool storeExifInfo = true);
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
               unsigned int stackOrder = 0 );
    virtual ~ImageInfo() { saveChanges(); }

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

    void rotate( int degrees, RotationMode mode=RotateImageInfoAndAreas );
    int angle() const;
    void setAngle( int angle );

    short rating() const;
    void setRating( short rating );

    bool isStacked() const { return m_stackId != 0; }
    StackID stackId() const;

    unsigned int stackOrder() const;
    void setStackOrder( const unsigned int stackOrder );

    void setVideoLength(int seconds);
    int videoLength() const;

    void setCategoryInfo( const QString& key,  const StringSet& value );
    void addCategoryInfo( const QString& category, const StringSet& values );
    /**
     * Enable a tag within a category for this image.
     * Optionally, the tag's position can be given (for positionable categories).
     * @param category the category name
     * @param value the tag name
     * @param area the image region that the tag applies to.
     */
    void addCategoryInfo(const QString& category, const QString& value, const QRect& area = QRect());
    void clearAllCategoryInfo();
    void removeCategoryInfo( const QString& category, const StringSet& values );
    void removeCategoryInfo( const QString& category, const QString& value );
    /**
     * Set the tagged areas for the image.
     * It is assumed that the positioned tags have already been set to the ImageInfo
     * using one of the functions <code>setCategoryInfo</code> or <code>addCategoryInfo</code>.
     *
     * @param category the category name.
     * @param positionedTags a mapping of tag names to image areas.
     */
    void setPositionedTags(const QString& category, const QMap<QString, QRect> &positionedTags);

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

    const MD5& MD5Sum() const { return m_md5sum; }
    void setMD5Sum( const MD5& sum, bool storeEXIF=true );

    void setLocked( bool );
    bool isLocked() const;

    bool isNull() const { return m_null; }
    QSize size() const;
    void setSize( const QSize& size );

    MediaType mediaType() const;
    void setMediaType( MediaType type ) { if (type != m_type) m_dirty = true; m_type = type; saveChangesIfNotDelayed(); }
    bool isVideo() const;

    void createFolderCategoryItem( DB::CategoryPtr, DB::MemberMap& memberMap );

    void delaySavingChanges(bool b=true);

    void copyExtraData( const ImageInfo& from, bool copyAngle = true);
    void removeExtraData();
    /**
     * Merge another ImageInfo into this one.
     * The other ImageInfo is not altered in any way or removed.
     */
    void merge(const ImageInfo& other);

    QMap<QString, QMap<QString, QRect>> taggedAreas() const;
    /**
     * Return the area associated with a tag.
     * @param category the category name
     * @param tag the tag name
     * @return the associated area, or <code>QRect()</code> if no association exists.
     */
    QRect areaForTag(QString category, QString tag) const;
#ifdef HAVE_KGEOMAP
    KGeoMap::GeoCoordinates coordinates() const;
#endif

protected:
    /** Save changes to database.
     *
     * Back-ends, which need changes to be instantly in database,
     * should override this.
     */
    virtual void saveChanges() {}

    void saveChangesIfNotDelayed() { if (!m_delaySaving) saveChanges(); }

    void setIsNull(bool b) { m_null = b; }
    bool isDirty() const { return m_dirty; }
    void setIsDirty(bool b)  { m_dirty = b; }
    bool updateDateInformation( int mode ) const;

    void setStackId( const StackID stackId );
    friend class XMLDB::Database;
private:
    DB::FileName m_fileName;
    QString m_label;
    QString m_description;
    ImageDate m_date;
    QMap<QString, StringSet> m_categoryInfomation;
    QMap<QString, QMap<QString, QRect>> m_taggedAreas;
    int m_angle;
    enum OnDisk { YesOnDisk, NoNotOnDisk, Unchecked };
    mutable OnDisk m_imageOnDisk;
    MD5 m_md5sum;
    bool m_null;
    QSize m_size;
    MediaType m_type;
    short m_rating;
    StackID m_stackId;
    unsigned int m_stackOrder;
    int m_videoLength;
#ifdef HAVE_KGEOMAP
    mutable KGeoMap::GeoCoordinates m_coordinates;
    mutable bool m_coordsIsSet = false;
#endif

    // Cache information
    bool m_locked;

    // Will be set to true after every change
    bool m_dirty;

    bool m_delaySaving;
};

}

#endif /* IMAGEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
