// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DescriptionUtil.h"
#include "Timespan.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpaexif/Info.h>

#include <KLocalizedString>
#include <QList>
#include <QTextCodec>
#include <QUrl>

/**
 * Add a line label + info text to the result text if info is not empty.
 * If the result already contains something, a HTML newline is added first.
 * To be used in createInfoText().
 */
static void AddNonEmptyInfo(const QString &label, const QString &infoText, QString *result)
{
    if (infoText.isEmpty()) {
        return;
    }
    if (!result->isEmpty()) {
        *result += QLatin1String("<br/>");
    }
    result->append(label).append(infoText);
}

/**
 * Given an ImageInfoPtr this function will create an HTML blob about the
 * image. The blob is used in the viewer and in the tool tip box from the
 * thumbnail view.
 *
 * As the HTML text is created, the parameter linkMap is filled with
 * information about hyperlinks. The map maps from an index to a pair of
 * (categoryName, categoryItem). This linkMap is used when the user selects
 * one of the hyberlinks.
 */
QString Utilities::createInfoText(DB::ImageInfoPtr info, QMap<int, QPair<QString, QString>> *linkMap)
{
    Q_ASSERT(info);

    QString result;
    if (Settings::SettingsData::instance()->showFilename()) {
        AddNonEmptyInfo(i18n("<b>File Name: </b> "), info->fileName().relative(), &result);
    }

    if (Settings::SettingsData::instance()->showDate()) {
        QString dateString = info->date().toString(Settings::SettingsData::instance()->showTime() ? true : false);
        dateString.append(Timespan::ago(info));
        AddNonEmptyInfo(i18n("<b>Date: </b> "), dateString, &result);
    }

    /* XXX */
    if (Settings::SettingsData::instance()->showImageSize() && info->mediaType() == DB::Image) {
        const QSize imageSize = info->size();
        // Do not add -1 x -1 text
        if (imageSize.width() >= 0 && imageSize.height() >= 0) {
            const double megapix = imageSize.width() * imageSize.height() / 1000000.0;
            QString infoText = i18nc("width x height", "%1x%2", QString::number(imageSize.width()), QString::number(imageSize.height()));
            if (megapix > 0.05) {
                infoText += i18nc("short for: x megapixels", " (%1MP)", QString::number(megapix, 'f', 1));
            }
            const double aspect = (double)imageSize.width() / (double)imageSize.height();
            // 0.995 - 1.005 can still be considered quadratic
            if (aspect > 1.005)
                infoText += i18nc("aspect ratio", " (%1:1)", QLocale::system().toString(aspect, 'f', 2));
            else if (aspect >= 0.995)
                infoText += i18nc("aspect ratio", " (1:1)");
            else
                infoText += i18nc("aspect ratio", " (1:%1)", QLocale::system().toString(1.0 / aspect, 'f', 2));
            AddNonEmptyInfo(i18n("<b>Image Size: </b> "), infoText, &result);
        }
    }

    if (Settings::SettingsData::instance()->showRating()) {
        if (info->rating() != -1) {
            if (!result.isEmpty()) {
                result += QLatin1String("<br/>");
            }
            QUrl rating;
            rating.setScheme(QLatin1String("kratingwidget"));
            // we don't use the host part, but if we don't set it, we can't use port:
            rating.setHost(QLatin1String("int"));
            rating.setPort(qMin(qMax(static_cast<short int>(0), info->rating()), static_cast<short int>(10)));
            result += QLatin1String("<img src=\"%1\"/>").arg(rating.toString(QUrl::None));
        }
    }

    const QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    int link = 0;
    for (const DB::CategoryPtr &category : categories) {
        const QString categoryName = category->name();
        if (category->doShow()) {
            StringSet items = info->itemsOfCategory(categoryName);

            if (DB::ImageDB::instance()->untaggedCategoryFeatureConfigured()
                && !Settings::SettingsData::instance()->untaggedImagesTagVisible()) {

                if (categoryName == Settings::SettingsData::instance()->untaggedCategory()) {
                    if (items.contains(Settings::SettingsData::instance()->untaggedTag())) {
                        items.remove(Settings::SettingsData::instance()->untaggedTag());
                    }
                }
            }

            if (!items.empty()) {
                QString title = QLatin1String("<b>%1: </b> ").arg(category->name());
                QString infoText;
                bool first = true;
                for (const QString &item : qAsConst(items)) {
                    if (first) {
                        first = false;
                    } else {
                        infoText += QLatin1String(", ");
                    }

                    if (linkMap) {
                        ++link;
                        (*linkMap)[link] = QPair<QString, QString>(categoryName, item);
                        infoText += QLatin1String("<a href=\"%1\">%2</a>").arg(link).arg(item);
                        infoText += Timespan::age(category, item, info);
                    } else {
                        infoText += item;
                    }
                }
                AddNonEmptyInfo(title, infoText, &result);
            }
        }
    }

    if (Settings::SettingsData::instance()->showLabel()) {
        AddNonEmptyInfo(i18n("<b>Label: </b> "), info->label(), &result);
    }

    if (Settings::SettingsData::instance()->showDescription() && !info->description().trimmed().isEmpty()) {
        AddNonEmptyInfo(i18n("<b>Description: </b> "), info->description(),
                        &result);
    }

    QString exifText;
    if (Settings::SettingsData::instance()->showEXIF()) {
        typedef QMap<QString, QStringList> ExifMap;
        typedef ExifMap::const_iterator ExifMapIterator;
        ExifMap exifMap = Exif::Info::instance()->infoForViewer(info->fileName(), Settings::SettingsData::instance()->iptcCharset());

        for (ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt) {
            if (exifIt.key().startsWith(QLatin1String("Exif."))) {
                for (QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt) {
                    QString exifName = exifIt.key().split(QChar::fromLatin1('.')).last();
                    AddNonEmptyInfo(QLatin1String("<b>%1: </b> ").arg(exifName),
                                    *valuesIt, &exifText);
                }
            }
        }

        QString iptcText;
        for (ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt) {
            if (!exifIt.key().startsWith(QLatin1String("Exif."))) {
                for (QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt) {
                    QString iptcName = exifIt.key().split(QChar::fromLatin1('.')).last();
                    AddNonEmptyInfo(QLatin1String("<b>%1: </b> ").arg(iptcName),
                                    *valuesIt, &iptcText);
                }
            }
        }

        if (!iptcText.isEmpty()) {
            if (exifText.isEmpty()) {
                exifText = iptcText;
            } else {
                exifText += QLatin1String("<hr/>") + iptcText;
            }
        }
    }

    if (!result.isEmpty() && !exifText.isEmpty()) {
        result += QLatin1String("<hr/>");
    }
    result += exifText;

    return result;
}
