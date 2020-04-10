/* Copyright (C) 2003-2020 Jesper K. Pedersen <blackie@kde.org>

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

#include "ImageInfo.h"

#include "Logging.h"

#include <DB/Category.h>
#include <DB/CategoryCollection.h>
#include <DB/CategoryPtr.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/MemberMap.h>
#include <MainWindow/DirtyIndicator.h>
#include <Map/GeoCoordinates.h>

#include <QFileInfo>
#include <QList>

#define KEXIV_ORIENTATION_UNSPECIFIED 0
#define KEXIV_ORIENTATION_NORMAL 1
#define KEXIV_ORIENTATION_HFLIP 2
#define KEXIV_ORIENTATION_ROT_180 3
#define KEXIV_ORIENTATION_VFLIP 4
#define KEXIV_ORIENTATION_ROT_90_HFLIP 5
#define KEXIV_ORIENTATION_ROT_90 6
#define KEXIV_ORIENTATION_ROT_90_VFLIP 7
#define KEXIV_ORIENTATION_ROT_270 8

/**
 * Convert a rotation in degrees to a KExiv2::ImageOrientation value.
 */
static int deg2KexivOrientation(int deg)
{
    deg = (deg + 360) % 360;
    ;
    switch (deg) {
    case 0:
        return KEXIV_ORIENTATION_NORMAL;
    case 90:
        return KEXIV_ORIENTATION_ROT_90;
    case 180:
        return KEXIV_ORIENTATION_ROT_180;
    case 270:
        return KEXIV_ORIENTATION_ROT_270;
    default:
        qCWarning(PluginsLog) << "Rotation of " << deg << "degrees can't be mapped to KExiv2::ImageOrientation value.";
        return KEXIV_ORIENTATION_UNSPECIFIED;
    }
}
/**
 * Convert a KExiv2::ImageOrientation value into a degrees angle.
 */
static int kexivOrientation2deg(int orient)
{
    switch (orient) {
    case KEXIV_ORIENTATION_NORMAL:
        return 0;
    case KEXIV_ORIENTATION_ROT_90:
        return 90;
    case KEXIV_ORIENTATION_ROT_180:
        return 280;
    case KEXIV_ORIENTATION_ROT_270:
        return 270;
    default:
        qCWarning(PluginsLog) << "KExiv2::ImageOrientation value " << orient << " not a pure rotation. Discarding orientation info.";
        return 0;
    }
}

Plugins::ImageInfo::ImageInfo(KIPI::Interface *interface, const QUrl &url)
    : KIPI::ImageInfoShared(interface, url)
{
    m_info = DB::ImageDB::instance()->info(DB::FileName::fromAbsolutePath(_url.path()));
}

QMap<QString, QVariant> Plugins::ImageInfo::attributes()
{
    if (m_info == nullptr) {
        // This can happen if we're trying to access an image that
        // has been deleted on-disc, but not yet the database
        return QMap<QString, QVariant>();
    }

    Q_ASSERT(m_info);
    QMap<QString, QVariant> res;

    res.insert(QString::fromLatin1("name"), QFileInfo(m_info->fileName().absolute()).baseName());
    res.insert(QString::fromLatin1("comment"), m_info->description());

    res.insert(QLatin1String("date"), m_info->date().start());
    res.insert(QLatin1String("dateto"), m_info->date().end());
    res.insert(QLatin1String("isexactdate"), m_info->date().start() == m_info->date().end());

    res.insert(QString::fromLatin1("orientation"), deg2KexivOrientation(m_info->angle()));
    res.insert(QString::fromLatin1("angle"), deg2KexivOrientation(m_info->angle())); // for compatibility with older versions. Now called orientation.

    res.insert(QString::fromLatin1("title"), m_info->label());

    res.insert(QString::fromLatin1("rating"), m_info->rating());

    // not supported:
    //res.insert(QString::fromLatin1("colorlabel"), xxx );
    //res.insert(QString::fromLatin1("picklabel"), xxx );

#ifdef HAVE_MARBLE
    Map::GeoCoordinates position = m_info->coordinates();
    if (position.hasCoordinates()) {
        res.insert(QString::fromLatin1("longitude"), QVariant(position.lon()));
        res.insert(QString::fromLatin1("latitude"), QVariant(position.lat()));
        if (position.hasAltitude())
            res.insert(QString::fromLatin1("altitude"), QVariant(position.alt()));
    }
#endif

    // Flickr plug-in expects the item tags, so we better give them.
    QString text;
    const QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    QStringList tags;
    QStringList tagspath;
    const QLatin1String sep("/");
    for (const DB::CategoryPtr &category : categories) {
        const QString categoryName = category->name();
        if (category->isSpecialCategory())
            continue;
        // I don't know why any categories except the above should be excluded
        //if ( category->doShow() ) {
        const Utilities::StringSet items = m_info->itemsOfCategory(categoryName);
        for (const QString &tag : items) {
            tags.append(tag);
            // digikam compatible tag path:
            // note: this produces a semi-flattened hierarchy.
            // instead of "Places/France/Paris" this will yield "Places/Paris"
            tagspath.append(categoryName + sep + tag);
        }
        //}
    }
    res.insert(QString::fromLatin1("tagspath"), tagspath);
    res.insert(QString::fromLatin1("keywords"), tags);
    res.insert(QString::fromLatin1("tags"), tags); // for compatibility with older versions. Now called keywords.

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
    if (m_info) {
        // official behaviour is to delete all officially supported attributes:
        QStringList attr;
        attr.append(QString::fromLatin1("comment"));
        attr.append(QString::fromLatin1("date"));
        attr.append(QString::fromLatin1("title"));
        attr.append(QString::fromLatin1("orientation"));
        attr.append(QString::fromLatin1("tagspath"));
        attr.append(QString::fromLatin1("rating"));
        attr.append(QString::fromLatin1("colorlabel"));
        attr.append(QString::fromLatin1("picklabel"));
        attr.append(QString::fromLatin1("gpslocation"));
        attr.append(QString::fromLatin1("copyrights"));
        delAttributes(attr);
    }
}

void Plugins::ImageInfo::addAttributes(const QMap<QString, QVariant> &amap)
{
    if (m_info && !amap.empty()) {
        QMap<QString, QVariant> map = amap;
        if (map.contains(QLatin1String("name"))) {
            // plugin renamed the item
            // TODO: implement this
            qCWarning(PluginsLog, "File renaming by kipi-plugin not supported.");
            //map.remove(QLatin1String("name"));
        }
        if (map.contains(QLatin1String("comment"))) {
            // is it save to do that? digikam seems to allow multiple comments on a single image
            // if a plugin assumes that it is adding a comment, not setting it, things might go badly...
            m_info->setDescription(map[QLatin1String("comment")].toString());
            map.remove(QLatin1String("comment"));
        }
        // note: this probably won't work as expected because according to the spec,
        // "isexactdate" is supposed to be readonly and therefore never set here:
        if (map.contains(QLatin1String("isexactdate")) && map.contains(QLatin1String("date"))) {
            m_info->setDate(DB::ImageDate(map[QLatin1String("date")].toDateTime()));
            map.remove(QLatin1String("date"));
        } else if (map.contains(QLatin1String("date")) && map.contains(QLatin1String("dateto"))) {
            m_info->setDate(DB::ImageDate(map[QLatin1String("date")].toDateTime(), map[QLatin1String("dateto")].toDateTime()));
            map.remove(QLatin1String("date"));
            map.remove(QLatin1String("dateto"));
        } else if (map.contains(QLatin1String("date"))) {
            m_info->setDate(DB::ImageDate(map[QLatin1String("date")].toDateTime()));
            map.remove(QLatin1String("date"));
        }
        if (map.contains(QLatin1String("angle"))) {
            qCWarning(PluginsLog, "Kipi-plugin uses deprecated attribute \"angle\".");
            m_info->setAngle(kexivOrientation2deg(map[QLatin1String("angle")].toInt()));
            map.remove(QLatin1String("angle"));
        }
        if (map.contains(QLatin1String("orientation"))) {
            m_info->setAngle(kexivOrientation2deg(map[QLatin1String("orientation")].toInt()));
            map.remove(QLatin1String("orientation"));
        }
        if (map.contains(QLatin1String("title"))) {
            m_info->setLabel(map[QLatin1String("title")].toString());
            map.remove(QLatin1String("title"));
        }
        if (map.contains(QLatin1String("rating"))) {
            m_info->setRating(map[QLatin1String("rating")].toInt());
            map.remove(QLatin1String("rating"));
        }
        if (map.contains(QLatin1String("tagspath"))) {
            const QStringList tagspaths = map[QLatin1String("tagspath")].toStringList();
            const DB::CategoryCollection *categories = DB::ImageDB::instance()->categoryCollection();
            DB::MemberMap &memberMap = DB::ImageDB::instance()->memberMap();
            for (const QString &path : tagspaths) {
                qCDebug(PluginsLog) << "Adding tags: " << path;
                QStringList tagpath = path.split(QLatin1String("/"), QString::SkipEmptyParts);
                // Note: maybe tagspaths with only one component or with unknown first component
                //  should be added to the "keywords"/"Events" category?
                if (tagpath.size() < 2) {
                    qCWarning(PluginsLog) << "Ignoring incompatible tag: " << path;
                    continue;
                }

                // first component is the category,
                const QString categoryName = tagpath.takeFirst();
                DB::CategoryPtr cat = categories->categoryForName(categoryName);
                if (cat) {
                    QString previousTag;
                    // last component is the tag:
                    // others define hierarchy:
                    for (const QString &currentTag : qAsConst(tagpath)) {
                        if (!cat->items().contains(currentTag)) {
                            qCDebug(PluginsLog) << "Adding tag " << currentTag << " to category " << categoryName;
                            // before we can use a tag, we have to add it
                            cat->addItem(currentTag);
                        }
                        if (!previousTag.isNull()) {
                            if (!memberMap.isGroup(categoryName, previousTag)) {
                                // create a group for the parent tag, so we can add a sub-category
                                memberMap.addGroup(categoryName, previousTag);
                            }
                            if (memberMap.canAddMemberToGroup(categoryName, previousTag, currentTag)) {
                                // make currentTag a member of the previousTag group
                                memberMap.addMemberToGroup(categoryName, previousTag, currentTag);
                            } else {
                                qCWarning(PluginsLog) << "Cannot make " << currentTag << " a subcategory of "
                                                      << categoryName << "/" << previousTag << "!";
                            }
                        }
                        previousTag = currentTag;
                    }
                    qCDebug(PluginsLog) << "Adding tag " << previousTag << " in category " << categoryName
                                        << " to image " << m_info->label();
                    // previousTag must be a valid category (see addItem() above...)
                    m_info->addCategoryInfo(categoryName, previousTag);
                } else {
                    qCWarning(PluginsLog) << "Unknown category: " << categoryName;
                }
            }
            map.remove(QLatin1String("tagspath"));
        }

        // remove read-only keywords:
        map.remove(QLatin1String("filesize"));
        map.remove(QLatin1String("isexactdate"));
        map.remove(QLatin1String("keywords"));
        map.remove(QLatin1String("tags"));
        map.remove(QLatin1String("altitude"));
        map.remove(QLatin1String("longitude"));
        map.remove(QLatin1String("latitude"));

        // colorlabel
        // picklabel
        // creators
        // credit
        // rights
        // source

        MainWindow::DirtyIndicator::markDirty();
        if (!map.isEmpty()) {
            qCWarning(PluginsLog) << "The following attributes are not (yet) supported by the KPhotoAlbum KIPI interface:" << map;
        }
    }
}

void Plugins::ImageInfo::delAttributes(const QStringList &attrs)
{
    if (m_info && !attrs.empty()) {
        QStringList delAttrs = attrs;
        if (delAttrs.contains(QLatin1String("comment"))) {
            m_info->setDescription(QString());
            delAttrs.removeAll(QLatin1String("comment"));
        }
        // not supported: date
        if (delAttrs.contains(QLatin1String("orientation")) || delAttrs.contains(QLatin1String("angle"))) {
            m_info->setAngle(0);
            delAttrs.removeAll(QLatin1String("orientation"));
            delAttrs.removeAll(QLatin1String("angle"));
        }
        if (delAttrs.contains(QLatin1String("rating"))) {
            m_info->setRating(-1);
            delAttrs.removeAll(QLatin1String("rating"));
        }
        if (delAttrs.contains(QLatin1String("title"))) {
            m_info->setLabel(QString());
            delAttrs.removeAll(QLatin1String("title"));
        }
        // TODO:
        // (colorlabel)
        // (picklabel)
        // copyrights
        // not supported: gpslocation
        if (delAttrs.contains(QLatin1String("tags")) || delAttrs.contains(QLatin1String("tagspath"))) {
            m_info->clearAllCategoryInfo();
            delAttrs.removeAll(QLatin1String("tags"));
            delAttrs.removeAll(QLatin1String("tagspath"));
        }
        MainWindow::DirtyIndicator::markDirty();
        if (!delAttrs.isEmpty()) {
            qCWarning(PluginsLog) << "The following attributes are not (yet) supported by the KPhotoAlbum KIPI interface:" << delAttrs;
        }
    }
}

void Plugins::ImageInfo::cloneData(ImageInfoShared *const other)
{
    ImageInfoShared::cloneData(other);
    if (m_info) {
        Plugins::ImageInfo *inf = static_cast<Plugins::ImageInfo *>(other);
        m_info->setDate(inf->m_info->date());
        MainWindow::DirtyIndicator::markDirty();
    }
}

bool Plugins::ImageInfo::isPositionAttribute(const QString &key)
{
    return (key == QString::fromLatin1("longitude") || key == QString::fromLatin1("latitude") || key == QString::fromLatin1("altitude") || key == QString::fromLatin1("positionPrecision"));
}

bool Plugins::ImageInfo::isCategoryAttribute(const QString &key)
{
    return (key != QString::fromLatin1("tags") && !isPositionAttribute(key));
}

// vi:expandtab:tabstop=4 shiftwidth=4:
