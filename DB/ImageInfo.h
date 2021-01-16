/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <kpabase/config-kpa-marble.h>

#include "CategoryPtr.h"
#include "ExifMode.h"
#include "ImageDate.h"
#include "MD5.h"

#ifdef HAVE_MARBLE
#include <Map/GeoCoordinates.h>
#endif
#include <kpabase/FileName.h>
#include <kpabase/StringSet.h>

#include <QHash>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>

namespace Plugins
{
class ImageInfo;
}

namespace XMLDB
{
class Database;
}

namespace DB
{
enum RotationMode {
    RotateImageInfoAndAreas,
    RotateImageInfoOnly
};

using Utilities::StringSet;
class MemberMap;

/**
 * @brief The FileInformation enum controls the behaviour of the ImageInfo constructor.
 * Depending on the value, metadata is read from the image file and optionally the Exif database is updated.
 */
enum class FileInformation {
    Ignore, ///< Do not read additional information from the image file.
    Read, ///< Read metadata from the image file, but do not update metadata in the Exif database.
    ReadAndUpdateExifDB ///< Read metadata from the image file and update the Exif database.
};

enum MediaType { Image = 0x01,
                 Video = 0x02 };
const MediaType anyMediaType = MediaType(Image | Video);
typedef unsigned int StackID;

typedef QHash<QString, QRect> PositionTags;
typedef QHashIterator<QString, QRect> PositionTagsIterator;
typedef QHash<QString, PositionTags> TaggedAreas;
typedef QHashIterator<QString, PositionTags> TaggedAreasIterator;
typedef QHash<QString, StringSet> CategoryInformation;

class ImageInfo : public QSharedData
{

public:
    /**
     * @brief ImageInfo constructs an empty ImageInfo.
     * An empty imageInfo can be detected by calling \c isNull().
     */
    ImageInfo();
    /**
     * @brief ImageInfo constructor to create an ImageInfo for a file.
     * This constructor is typically called by the new image finder.
     * @param fileName
     * @param type
     * @param infoMode
     */
    explicit ImageInfo(const DB::FileName &fileName, MediaType type = Image, FileInformation infoMode = FileInformation::ReadAndUpdateExifDB);
    /**
     * @brief ImageInfo constructor including all fields.
     * This constructor is typically called when reading ImageInfos from the database file, or when doing an import.
     * @param fileName
     * @param label
     * @param description
     * @param date
     * @param angle
     * @param md5sum
     * @param size
     * @param type
     * @param rating
     * @param stackId
     * @param stackOrder
     */
    ImageInfo(const DB::FileName &fileName,
              const QString &label,
              const QString &description,
              const ImageDate &date,
              int angle,
              const MD5 &md5sum,
              const QSize &size,
              MediaType type,
              short rating = -1,
              StackID stackId = 0,
              unsigned int stackOrder = 0);
    ImageInfo(const ImageInfo &other);

    FileName fileName() const;
    void setFileName(const DB::FileName &relativeFileName);

    void setLabel(const QString &);
    QString label() const;

    void setDescription(const QString &);
    QString description() const;

    void setDate(const ImageDate &);
    ImageDate date() const;
    ImageDate &date();
    void readExif(const DB::FileName &fullPath, DB::ExifMode mode);

    void rotate(int degrees, RotationMode mode = RotateImageInfoAndAreas);
    int angle() const;
    void setAngle(int angle);

    short rating() const;
    void setRating(short rating);

    bool isStacked() const { return m_stackId != 0; }
    StackID stackId() const;

    unsigned int stackOrder() const;
    void setStackOrder(const unsigned int stackOrder);

    void setVideoLength(int seconds);
    int videoLength() const;

    void setCategoryInfo(const QString &key, const StringSet &value);
    void addCategoryInfo(const QString &category, const StringSet &values);
    /**
     * Enable a tag within a category for this image.
     * Optionally, the tag's position can be given (for positionable categories).
     * @param category the category name
     * @param value the tag name
     * @param area the image region that the tag applies to.
     */
    void addCategoryInfo(const QString &category, const QString &value, const QRect &area = QRect());
    void clearAllCategoryInfo();
    void removeCategoryInfo(const QString &category, const StringSet &values);
    void removeCategoryInfo(const QString &category, const QString &value);
    /**
     * Set the tagged areas for the image.
     * It is assumed that the positioned tags have already been set to the ImageInfo
     * using one of the functions <code>setCategoryInfo</code> or <code>addCategoryInfo</code>.
     *
     * @param category the category name.
     * @param positionedTags a mapping of tag names to image areas.
     */
    void setPositionedTags(const QString &category, const PositionTags &positionedTags);

    bool hasCategoryInfo(const QString &key, const QString &value) const;
    bool hasCategoryInfo(const QString &key, const StringSet &values) const;

    QStringList availableCategories() const;
    StringSet itemsOfCategory(const QString &category) const;
    void renameItem(const QString &key, const QString &oldValue, const QString &newValue);
    void renameCategory(const QString &oldName, const QString &newName);

    bool operator!=(const ImageInfo &other) const;
    bool operator==(const ImageInfo &other) const;
    ImageInfo &operator=(const ImageInfo &other);

    static bool imageOnDisk(const DB::FileName &fileName);

    const MD5 &MD5Sum() const { return m_md5sum; }
    void setMD5Sum(const MD5 &sum, bool storeEXIF = true);

    void setLocked(bool);
    bool isLocked() const;

    bool isNull() const { return m_null; }
    QSize size() const;
    void setSize(const QSize &size);

    MediaType mediaType() const;
    void setMediaType(MediaType type)
    {
        if (type != m_type)
            m_dirty = true;
        m_type = type;
    }
    bool isVideo() const;

    void createFolderCategoryItem(DB::CategoryPtr, DB::MemberMap &memberMap);

    void copyExtraData(const ImageInfo &from, bool copyAngle = true);
    void removeExtraData();
    /**
     * Merge another ImageInfo into this one.
     * The other ImageInfo is not altered in any way or removed.
     */
    void merge(const ImageInfo &other);

    TaggedAreas taggedAreas() const;
    /**
     * Return the area associated with a tag.
     * @param category the category name
     * @param tag the tag name
     * @return the associated area, or <code>QRect()</code> if no association exists.
     */
    QRect areaForTag(QString category, QString tag) const;
    void setIsMatched(bool isMatched);
    bool isMatched() const;
    void setMatchGeneration(int matchGeneration);
    int matchGeneration() const;
#ifdef HAVE_MARBLE
    Map::GeoCoordinates coordinates() const;
#endif

protected:
    void setIsNull(bool b) { m_null = b; }
    bool isDirty() const { return m_dirty; }
    void markDirty();
    bool updateDateInformation(int mode) const;

    void setStackId(const StackID stackId);
    friend class XMLDB::Database;

private:
    DB::FileName m_fileName;
    QString m_label;
    QString m_description;
    ImageDate m_date;
    CategoryInformation m_categoryInfomation;
    TaggedAreas m_taggedAreas;
    int m_angle;
    enum OnDisk { YesOnDisk,
                  NoNotOnDisk,
                  Unchecked };
    mutable OnDisk m_imageOnDisk;
    MD5 m_md5sum;
    bool m_null;
    QSize m_size;
    MediaType m_type;
    short m_rating;
    StackID m_stackId;
    unsigned int m_stackOrder;
    int m_videoLength;
    bool m_isMatched;
    int m_matchGeneration;
#ifdef HAVE_MARBLE
    mutable Map::GeoCoordinates m_coordinates;
    mutable bool m_coordsIsSet = false;
#endif

    // Cache information
    bool m_locked;

    // Will be set to true after every change
    bool m_dirty;
};
}

#endif /* IMAGEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
