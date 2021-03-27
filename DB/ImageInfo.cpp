// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageInfo.h"

#include "CategoryCollection.h"
#include "FileInfo.h"
#include "ImageDB.h"
#include "MemberMap.h"

#include <kpabase/FileNameUtil.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpabase/StringSet.h>
#include <kpaexif/Database.h>
#include <kpaexif/DatabaseElement.h>

#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QStringList>

using namespace DB;

ImageInfo::ImageInfo()
    : m_null(true)
    , m_rating(-1)
    , m_stackId(0)
    , m_stackOrder(0)
    , m_videoLength(-1)
    , m_isMatched(false)
    , m_matchGeneration(-1)
    , m_locked(false)
    , m_dirty(false)
{
}

ImageInfo::ImageInfo(const DB::FileName &fileName, MediaType type, FileInformation infoMode)
    : m_imageOnDisk(YesOnDisk)
    , m_null(false)
    , m_size(-1, -1)
    , m_type(type)
    , m_rating(-1)
    , m_stackId(0)
    , m_stackOrder(0)
    , m_videoLength(-1)
    , m_isMatched(false)
    , m_matchGeneration(-1)
    , m_locked(false)
{
    QFileInfo fi(fileName.absolute());
    m_label = fi.completeBaseName();
    m_angle = 0;

    setFileName(fileName);

    // Read Exif information
    if (infoMode != FileInformation::Ignore) {
        ExifMode mode = EXIFMODE_INIT;
        if (infoMode == FileInformation::ReadAndUpdateExifDB)
            mode &= ~EXIFMODE_DATABASE_UPDATE;
        readExif(fileName, mode);
    }

    m_dirty = false;
}

ImageInfo::ImageInfo(const ImageInfo &other)
    : QSharedData(other)
{
    *this = other;
}

void ImageInfo::setIsMatched(bool isMatched)
{
    m_isMatched = isMatched;
}

bool ImageInfo::isMatched() const
{
    return m_isMatched;
}

void ImageInfo::setMatchGeneration(int matchGeneration)
{
    m_matchGeneration = matchGeneration;
}

int ImageInfo::matchGeneration() const
{
    return m_matchGeneration;
}

void ImageInfo::setLabel(const QString &desc)
{
    if (desc != m_label)
        markDirty();
    m_label = desc;
}

QString ImageInfo::label() const
{
    return m_label;
}

void ImageInfo::setDescription(const QString &desc)
{
    if (desc != m_description)
        markDirty();
    m_description = desc.trimmed();
}

QString ImageInfo::description() const
{
    return m_description;
}

void ImageInfo::setCategoryInfo(const QString &key, const StringSet &value)
{
    // Don't check if really changed, because it's too slow.
    markDirty();
    m_categoryInfomation[key] = value;
}

bool ImageInfo::hasCategoryInfo(const QString &key, const QString &value) const
{
    return m_categoryInfomation[key].contains(value);
}

bool DB::ImageInfo::hasCategoryInfo(const QString &key, const StringSet &values) const
{
    return values.intersects(m_categoryInfomation[key]);
}

StringSet ImageInfo::itemsOfCategory(const QString &key) const
{
    return m_categoryInfomation[key];
}

void ImageInfo::renameItem(const QString &category, const QString &oldValue, const QString &newValue)
{
    if (m_taggedAreas.contains(category)) {
        if (m_taggedAreas[category].contains(oldValue)) {
            m_taggedAreas[category][newValue] = m_taggedAreas[category][oldValue];
            m_taggedAreas[category].remove(oldValue);
        }
    }

    StringSet &set = m_categoryInfomation[category];
    StringSet::iterator it = set.find(oldValue);
    if (it != set.end()) {
        markDirty();
        set.erase(it);
        set.insert(newValue);
    }
}

DB::FileName ImageInfo::fileName() const
{
    return m_fileName;
}

void ImageInfo::setFileName(const DB::FileName &fileName)
{
    if (fileName != m_fileName)
        markDirty();
    m_fileName = fileName;

    m_imageOnDisk = Unchecked;
    DB::CategoryPtr folderCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::FolderCategory);
    if (folderCategory) {
        DB::MemberMap &map = DB::ImageDB::instance()->memberMap();
        createFolderCategoryItem(folderCategory, map);
        //ImageDB::instance()->setMemberMap( map );
    }
}

void ImageInfo::rotate(int degrees, RotationMode mode)
{
    // ensure positive degrees:
    degrees += 360;
    degrees = degrees % 360;
    if (degrees == 0)
        return;

    markDirty();
    m_angle = (m_angle + degrees) % 360;

    if (degrees == 90 || degrees == 270) {
        m_size.transpose();
    }

    // the AnnotationDialog manages this by itself and sets RotateImageInfoOnly:
    if (mode == RotateImageInfoAndAreas) {
        for (auto &areasOfCategory : m_taggedAreas) {
            for (auto &area : areasOfCategory) {
                QRect rotatedArea;

                // parameter order for QRect::setCoords:
                // setCoords( left, top, right, bottom )
                // keep in mind that _size is already transposed
                switch (degrees) {
                case 90:
                    rotatedArea.setCoords(
                        m_size.width() - area.bottom(),
                        area.left(),
                        m_size.width() - area.top(),
                        area.right());
                    break;
                case 180:
                    rotatedArea.setCoords(
                        m_size.width() - area.right(),
                        m_size.height() - area.bottom(),
                        m_size.width() - area.left(),
                        m_size.height() - area.top());
                    break;
                case 270:
                    rotatedArea.setCoords(
                        area.top(),
                        m_size.height() - area.right(),
                        area.bottom(),
                        m_size.height() - area.left());
                    break;
                default:
                    // degrees==0; "odd" values won't happen.
                    rotatedArea = area;
                    break;
                }

                // update _taggedAreas[category][tag]:
                area = rotatedArea;
            }
        }
    }
}

int ImageInfo::angle() const
{
    return m_angle;
}

void ImageInfo::setAngle(int angle)
{
    if (angle != m_angle)
        markDirty();
    m_angle = angle;
}

short ImageInfo::rating() const
{
    return m_rating;
}

void ImageInfo::setRating(short rating)
{
    Q_ASSERT((rating >= 0 && rating <= 10) || rating == -1);

    if (rating > 10)
        rating = 10;
    if (rating < -1)
        rating = -1;
    if (m_rating != rating)
        markDirty();

    m_rating = rating;
}

DB::StackID ImageInfo::stackId() const
{
    return m_stackId;
}

void ImageInfo::setStackId(const DB::StackID stackId)
{
    if (stackId != m_stackId)
        markDirty();
    m_stackId = stackId;
}

unsigned int ImageInfo::stackOrder() const
{
    return m_stackOrder;
}

void ImageInfo::setStackOrder(const unsigned int stackOrder)
{
    if (stackOrder != m_stackOrder)
        markDirty();
    m_stackOrder = stackOrder;
}

void ImageInfo::setVideoLength(int length)
{
    if (m_videoLength != length)
        markDirty();
    m_videoLength = length;
}

int ImageInfo::videoLength() const
{
    return m_videoLength;
}

void ImageInfo::setDate(const ImageDate &date)
{
    if (date != m_date)
        markDirty();
    m_date = date;
}

ImageDate &ImageInfo::date()
{
    return m_date;
}

ImageDate ImageInfo::date() const
{
    return m_date;
}

bool ImageInfo::operator!=(const ImageInfo &other) const
{
    return !(*this == other);
}

bool ImageInfo::operator==(const ImageInfo &other) const
{
    bool changed = (m_fileName != other.m_fileName || m_label != other.m_label || (!m_description.isEmpty() && !other.m_description.isEmpty() && m_description != other.m_description) || // one might be isNull.
                    m_date != other.m_date || m_angle != other.m_angle || m_rating != other.m_rating || (m_stackId != other.m_stackId || !((m_stackId == 0) ? true : (m_stackOrder == other.m_stackOrder))));
    if (!changed) {
        QStringList keys = DB::ImageDB::instance()->categoryCollection()->categoryNames();
        for (QStringList::ConstIterator it = keys.constBegin(); it != keys.constEnd(); ++it)
            changed |= m_categoryInfomation[*it] != other.m_categoryInfomation[*it];
    }
    return !changed;
}

void ImageInfo::renameCategory(const QString &oldName, const QString &newName)
{
    markDirty();

    m_categoryInfomation[newName] = m_categoryInfomation[oldName];
    m_categoryInfomation.remove(oldName);

    m_taggedAreas[newName] = m_taggedAreas[oldName];
    m_taggedAreas.remove(oldName);
}

void ImageInfo::setMD5Sum(const MD5 &sum, bool storeEXIF)
{
    if (sum != m_md5sum) {
        // if we make a QObject derived class out of imageinfo, we might invalidate thumbnails from here

        // file changed -> reload/invalidate metadata:
        ExifMode mode = EXIFMODE_ORIENTATION | EXIFMODE_DATABASE_UPDATE;
        // fuzzy dates are usually set for a reason
        if (!m_date.isFuzzy())
            mode |= EXIFMODE_DATE;
        // FIXME (ZaJ): the "right" thing to do would be to update the description
        //              - if it is currently empty (done.)
        //              - if it has been set from the exif info and not been changed (TODO)
        if (m_description.isEmpty())
            mode |= EXIFMODE_DESCRIPTION;

        if (!storeEXIF)
            mode &= ~EXIFMODE_DATABASE_UPDATE;
        readExif(fileName(), mode);
        if (storeEXIF) {
            // Size isn't really EXIF, but this is the most obvious
            // place to extract it
            QImageReader reader(m_fileName.absolute());
            if (reader.canRead()) {
                m_size = reader.size();
                if (m_angle == 90 || m_angle == 270)
                    m_size.transpose();
            }
        }

        // FIXME (ZaJ): it *should* make sense to set the ImageDB::md5Map() from here, but I want
        //              to make sure I fully understand everything first...
        //              this could also be done as signal md5Changed(old,new)

        // image size is invalidated by the thumbnail builder, if needed

        markDirty();
    }
    m_md5sum = sum;
}

void ImageInfo::setLocked(bool locked)
{
    m_locked = locked;
}

bool ImageInfo::isLocked() const
{
    return m_locked;
}

void ImageInfo::readExif(const DB::FileName &fullPath, DB::ExifMode mode)
{
    DB::FileInfo exifInfo = DB::FileInfo::read(fullPath, mode);

    // Date
    if (updateDateInformation(mode)) {
        const ImageDate newDate(exifInfo.dateTime());
        setDate(newDate);
    }

    // Orientation
    if ((mode & EXIFMODE_ORIENTATION) && Settings::SettingsData::instance()->useEXIFRotate()) {
        setAngle(exifInfo.angle());
    }

    // Description
    if ((mode & EXIFMODE_DESCRIPTION) && Settings::SettingsData::instance()->useEXIFComments()) {
        bool doSetDescription = true;
        QString desc = exifInfo.description();

        if (Settings::SettingsData::instance()->stripEXIFComments()) {
            for (const auto &ignoredComment : Settings::SettingsData::instance()->EXIFCommentsToStrip()) {
                if (desc == ignoredComment) {
                    doSetDescription = false;
                    break;
                }
            }
        }

        if (doSetDescription) {
            setDescription(desc);
        }
    }

    // Database update
    if (mode & EXIFMODE_DATABASE_UPDATE) {
        DB::ImageDB::instance()->exifDB()->add(exifInfo.getFileName(), exifInfo.getExifData());
#ifdef HAVE_MARBLE
        // GPS coords might have changed...
        m_coordsIsSet = false;
#endif
    }
}

QStringList ImageInfo::availableCategories() const
{
    return m_categoryInfomation.keys();
}

QSize ImageInfo::size() const
{
    return m_size;
}

void ImageInfo::setSize(const QSize &size)
{
    if (size != m_size)
        markDirty();
    m_size = size;
}

bool ImageInfo::imageOnDisk(const DB::FileName &fileName)
{
    return fileName.exists();
}

ImageInfo::ImageInfo(const DB::FileName &fileName,
                     const QString &label,
                     const QString &description,
                     const ImageDate &date,
                     int angle,
                     const MD5 &md5sum,
                     const QSize &size,
                     MediaType type,
                     short rating,
                     unsigned int stackId,
                     unsigned int stackOrder)
{
    m_fileName = fileName;
    m_label = label;
    m_description = description;
    m_date = date;
    m_angle = angle;
    m_md5sum = md5sum;
    m_size = size;
    m_imageOnDisk = Unchecked;
    m_locked = false;
    m_null = false;
    m_type = type;
    markDirty();

    if (rating > 10)
        rating = 10;
    if (rating < -1)
        rating = -1;
    m_rating = rating;
    m_stackId = stackId;
    m_stackOrder = stackOrder;
    m_videoLength = -1;
    m_matchGeneration = -1;
}

// Note: we need this operator because the base class QSharedData hides
// its copy operator to make exclude the reference counting from being
// copied.
ImageInfo &ImageInfo::operator=(const ImageInfo &other)
{
    m_fileName = other.m_fileName;
    m_label = other.m_label;
    m_description = other.m_description;
    m_date = other.m_date;
    m_categoryInfomation = other.m_categoryInfomation;
    m_taggedAreas = other.m_taggedAreas;
    m_angle = other.m_angle;
    m_imageOnDisk = other.m_imageOnDisk;
    m_md5sum = other.m_md5sum;
    m_null = other.m_null;
    m_size = other.m_size;
    m_type = other.m_type;
    m_rating = other.m_rating;
    m_stackId = other.m_stackId;
    m_stackOrder = other.m_stackOrder;
    m_videoLength = other.m_videoLength;
    m_isMatched = other.m_isMatched;
    m_matchGeneration = other.m_matchGeneration;
#ifdef HAVE_KGEOMAP
    m_coordinates = other.m_coordinates;
    m_coordsIsSet = other.m_coordsIsSet;
#endif
    m_locked = other.m_locked;
    m_dirty = other.m_dirty;

    return *this;
}

MediaType DB::ImageInfo::mediaType() const
{
    return m_type;
}

bool ImageInfo::isVideo() const
{
    return m_type == Video;
}

void DB::ImageInfo::createFolderCategoryItem(DB::CategoryPtr folderCategory, DB::MemberMap &memberMap)
{
    QString folderName = Utilities::relativeFolderName(m_fileName.relative());
    if (folderName.isEmpty())
        return;

    if (!memberMap.contains(folderCategory->name(), folderName)) {
        QStringList directories = folderName.split(QString::fromLatin1("/"));

        QString curPath;
        for (QStringList::ConstIterator directoryIt = directories.constBegin(); directoryIt != directories.constEnd(); ++directoryIt) {
            if (curPath.isEmpty())
                curPath = *directoryIt;
            else {
                QString oldPath = curPath;
                curPath = curPath + QString::fromLatin1("/") + *directoryIt;
                memberMap.addMemberToGroup(folderCategory->name(), oldPath, curPath);
            }
        }
        folderCategory->addItem(folderName);
    }

    m_categoryInfomation.insert(folderCategory->name(), StringSet() << folderName);
}

void DB::ImageInfo::copyExtraData(const DB::ImageInfo &from, bool copyAngle)
{
    m_categoryInfomation = from.m_categoryInfomation;
    m_description = from.m_description;
    // Hmm...  what should the date be?  orig or modified?
    // _date = from._date;
    if (copyAngle)
        m_angle = from.m_angle;
    m_rating = from.m_rating;
}

void DB::ImageInfo::removeExtraData()
{
    m_categoryInfomation.clear();
    m_description.clear();
    m_rating = -1;
}

void ImageInfo::merge(const ImageInfo &other)
{
    // Merge date
    if (other.date() != m_date) {
        // a fuzzy date has been set by the user and therefore "wins" over an exact date.
        // two fuzzy dates can be merged
        // two exact dates should ideally be cross-checked with Exif information in the file.
        // Nevertheless, we merge them into a fuzzy date to avoid the complexity of checking the file.
        if (other.date().isFuzzy()) {
            if (m_date.isFuzzy())
                m_date.extendTo(other.date());
            else
                m_date = other.date();
        } else if (!m_date.isFuzzy()) {
            m_date.extendTo(other.date());
        }
        // else: keep m_date
    }

    // Merge description
    if (!other.description().isEmpty()) {
        if (m_description.isEmpty())
            m_description = other.description();
        else if (m_description != other.description())
            m_description += QString::fromUtf8("\n-----------\n") + other.m_description;
    }

    // Clear untagged tag if only one of the images was untagged
    const QString untaggedCategory = Settings::SettingsData::instance()->untaggedCategory();
    const QString untaggedTag = Settings::SettingsData::instance()->untaggedTag();
    const bool isCompleted = !m_categoryInfomation[untaggedCategory].contains(untaggedTag) || !other.m_categoryInfomation[untaggedCategory].contains(untaggedTag);

    // Merge tags
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const auto categoryInfomationKeys = m_categoryInfomation.keys();
    QSet<QString> keys(categoryInfomationKeys.begin(), categoryInfomationKeys.end());
    const auto otherCategoryInfomationKeys = other.m_categoryInfomation.keys();
    const QSet<QString> otherCategoryInfomationKeysSet(otherCategoryInfomationKeys.begin(), otherCategoryInfomationKeys.end());
#else
    QSet<QString> keys = QSet<QString>::fromList(m_categoryInfomation.keys());
    const auto otherCategoryInfomationKeysSet = QSet<QString>::fromList(other.m_categoryInfomation.keys());
#endif
    keys.unite(otherCategoryInfomationKeysSet);
    for (const QString &key : keys) {
        m_categoryInfomation[key].unite(other.m_categoryInfomation[key]);
    }

    // Clear untagged tag if only one of the images was untagged
    if (isCompleted)
        m_categoryInfomation[untaggedCategory].remove(untaggedTag);

    // merge stacks:
    if (isStacked() || other.isStacked()) {
        DB::FileNameList stackImages;
        if (!isStacked())
            stackImages.append(fileName());
        else
            stackImages.append(DB::ImageDB::instance()->getStackFor(fileName()));
        stackImages.append(DB::ImageDB::instance()->getStackFor(other.fileName()));

        DB::ImageDB::instance()->unstack(stackImages);
        if (!DB::ImageDB::instance()->stack(stackImages))
            qCWarning(DBLog, "Could not merge stacks!");
    }
}

void DB::ImageInfo::addCategoryInfo(const QString &category, const StringSet &values)
{
    for (StringSet::const_iterator valueIt = values.constBegin(); valueIt != values.constEnd(); ++valueIt) {
        if (!m_categoryInfomation[category].contains(*valueIt)) {
            markDirty();
            m_categoryInfomation[category].insert(*valueIt);
        }
    }
}

void DB::ImageInfo::clearAllCategoryInfo()
{
    m_categoryInfomation.clear();
    m_taggedAreas.clear();
}

void DB::ImageInfo::removeCategoryInfo(const QString &category, const StringSet &values)
{
    for (StringSet::const_iterator valueIt = values.constBegin(); valueIt != values.constEnd(); ++valueIt) {
        if (m_categoryInfomation[category].contains(*valueIt)) {
            markDirty();
            m_categoryInfomation[category].remove(*valueIt);
            m_taggedAreas[category].remove(*valueIt);
        }
    }
}

void DB::ImageInfo::addCategoryInfo(const QString &category, const QString &value, const QRect &area)
{
    if (!m_categoryInfomation[category].contains(value)) {
        markDirty();
        m_categoryInfomation[category].insert(value);

        if (area.isValid()) {
            m_taggedAreas[category][value] = area;
        }
    }
}

void DB::ImageInfo::removeCategoryInfo(const QString &category, const QString &value)
{
    if (m_categoryInfomation[category].contains(value)) {
        markDirty();
        m_categoryInfomation[category].remove(value);
        m_taggedAreas[category].remove(value);
    }
}

void DB::ImageInfo::setPositionedTags(const QString &category, const PositionTags &positionedTags)
{
    markDirty();
    m_taggedAreas[category] = positionedTags;
}

bool DB::ImageInfo::updateDateInformation(int mode) const
{
    if ((mode & EXIFMODE_DATE) == 0)
        return false;

    if ((mode & EXIFMODE_FORCE) != 0)
        return true;

    return true;
}

TaggedAreas DB::ImageInfo::taggedAreas() const
{
    return m_taggedAreas;
}

QRect DB::ImageInfo::areaForTag(QString category, QString tag) const
{
    // QMap::value returns a default constructed value if the key is not found:
    return m_taggedAreas.value(category).value(tag);
}

#ifdef HAVE_MARBLE
Map::GeoCoordinates DB::ImageInfo::coordinates() const
{
    if (m_coordsIsSet) {
        return m_coordinates;
    }

    static const int EXIF_GPS_VERSIONID = 0;
    static const int EXIF_GPS_LATREF = 1;
    static const int EXIF_GPS_LAT = 2;
    static const int EXIF_GPS_LONREF = 3;
    static const int EXIF_GPS_LON = 4;
    static const int EXIF_GPS_ALTREF = 5;
    static const int EXIF_GPS_ALT = 6;

    static const QString S = QString::fromUtf8("S");
    static const QString W = QString::fromUtf8("W");

    static QList<Exif::DatabaseElement *> fields;
    if (fields.isEmpty()) {
        // the order here matters! we use the named int constants afterwards to refer to them:
        fields.append(new Exif::IntExifElement("Exif.GPSInfo.GPSVersionID")); // actually a byte value
        fields.append(new Exif::StringExifElement("Exif.GPSInfo.GPSLatitudeRef"));
        fields.append(new Exif::RationalExifElement("Exif.GPSInfo.GPSLatitude"));
        fields.append(new Exif::StringExifElement("Exif.GPSInfo.GPSLongitudeRef"));
        fields.append(new Exif::RationalExifElement("Exif.GPSInfo.GPSLongitude"));
        fields.append(new Exif::IntExifElement("Exif.GPSInfo.GPSAltitudeRef")); // actually a byte value
        fields.append(new Exif::RationalExifElement("Exif.GPSInfo.GPSAltitude"));
    }

    // read field values from database:
    bool foundIt = DB::ImageDB::instance()->exifDB()->readFields(m_fileName, fields);

    // if the Database query result doesn't contain exif GPS info (-> upgraded exifdb from DBVersion < 2), it is null
    // if the result is int 0, then there's no exif gps information in the image
    // otherwise we can proceed to parse the information
    if (foundIt && fields[EXIF_GPS_VERSIONID]->value().isNull()) {
        auto exifDB = DB::ImageDB::instance()->exifDB();
        // update exif DB and repeat the search:
        exifDB->remove(fileName());
        exifDB->add(fileName());

        exifDB->readFields(m_fileName, fields);
        Q_ASSERT(!fields[EXIF_GPS_VERSIONID]->value().isNull());
    }

    Map::GeoCoordinates coords;

    // gps info set?
    // don't use the versionid field here, because some cameras use 0 as its value
    if (foundIt && fields[EXIF_GPS_LAT]->value().toInt() != -1.0
        && fields[EXIF_GPS_LON]->value().toInt() != -1.0) {
        // lat/lon/alt reference determines sign of float:
        double latr = (fields[EXIF_GPS_LATREF]->value().toString() == S) ? -1.0 : 1.0;
        double lat = fields[EXIF_GPS_LAT]->value().toFloat();
        double lonr = (fields[EXIF_GPS_LONREF]->value().toString() == W) ? -1.0 : 1.0;
        double lon = fields[EXIF_GPS_LON]->value().toFloat();
        double altr = (fields[EXIF_GPS_ALTREF]->value().toInt() == 1) ? -1.0 : 1.0;
        double alt = fields[EXIF_GPS_ALT]->value().toFloat();

        if (lat != -1.0 && lon != -1.0) {
            coords.setLatLon(latr * lat, lonr * lon);
            if (alt != 0.0f) {
                coords.setAlt(altr * alt);
            }
        }
    }

    m_coordinates = coords;
    m_coordsIsSet = true;
    return m_coordinates;
}

#endif

void ImageInfo::markDirty()
{
    m_dirty = true;
    m_matchGeneration = -1;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
