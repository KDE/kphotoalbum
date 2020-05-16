/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

// Local includes
#include "DescriptionUtil.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "Exif/Info.h"
#include "Logging.h"
#include "Settings/SettingsData.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QList>
#include <QTextCodec>
#include <QUrl>

namespace
{
const QLatin1String LINE_BREAK("<br/>");
}

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
        *result += LINE_BREAK;
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
        dateString.append(timeAgo(info));
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
            if (!result.isEmpty())
                result += QString::fromLatin1("<br/>");
            QUrl rating;
            rating.setScheme(QString::fromLatin1("kratingwidget"));
            // we don't use the host part, but if we don't set it, we can't use port:
            rating.setHost(QString::fromLatin1("int"));
            rating.setPort(qMin(qMax(static_cast<short int>(0), info->rating()), static_cast<short int>(10)));
            result += QString::fromLatin1("<img src=\"%1\"/>").arg(rating.toString(QUrl::None));
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
                QString title = QString::fromUtf8("<b>%1: </b> ").arg(category->name());
                QString infoText;
                bool first = true;
                for (const QString &item : qAsConst(items)) {
                    if (first)
                        first = false;
                    else
                        infoText += QString::fromLatin1(", ");

                    if (linkMap) {
                        ++link;
                        (*linkMap)[link] = QPair<QString, QString>(categoryName, item);
                        infoText += QString::fromLatin1("<a href=\"%1\">%2</a>").arg(link).arg(item);
                        infoText += formatAge(category, item, info);
                    } else
                        infoText += item;
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
            if (exifIt.key().startsWith(QString::fromLatin1("Exif.")))
                for (QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt) {
                    QString exifName = exifIt.key().split(QChar::fromLatin1('.')).last();
                    AddNonEmptyInfo(QString::fromLatin1("<b>%1: </b> ").arg(exifName),
                                    *valuesIt, &exifText);
                }
        }

        QString iptcText;
        for (ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt) {
            if (!exifIt.key().startsWith(QString::fromLatin1("Exif.")))
                for (QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt) {
                    QString iptcName = exifIt.key().split(QChar::fromLatin1('.')).last();
                    AddNonEmptyInfo(QString::fromLatin1("<b>%1: </b> ").arg(iptcName),
                                    *valuesIt, &iptcText);
                }
        }

        if (!iptcText.isEmpty()) {
            if (exifText.isEmpty())
                exifText = iptcText;
            else
                exifText += QString::fromLatin1("<hr>") + iptcText;
        }
    }

    if (!result.isEmpty() && !exifText.isEmpty())
        result += QString::fromLatin1("<hr>");
    result += exifText;

    return result;
}

namespace
{
enum class TimeUnit {
    /** Denotes a negative age. */
    Invalid,
    Days,
    Months,
    Years
};
class AgeSpec
{
public:
    /**
     * @brief The I18nContext enum determines how an age is displayed.
     */
    enum class I18nContext {
        /// For birthdays, e.g. "Jesper was 30 years in this image".
        Birthday,
        /// For ages of events, e.g. "This image was taken 30 years ago".
        Anniversary
    };
    int age; ///< The number of \c units, e.g. the "5" in "5 days"
    TimeUnit unit;

    AgeSpec();
    AgeSpec(int age, TimeUnit unit);

    /**
     * @brief format
     * @param context the context where the formatted age is used.
     * @return a localized string describing the time range.
     */
    QString format(I18nContext context) const;
    /**
     * @brief isValid
     * @return \c true, if the AgeSpec contains a valid age that is not negative. \c false otherwise.
     */
    bool isValid() const;
    bool operator==(const AgeSpec &other) const;
};

AgeSpec::AgeSpec()
    : age(70)
    , unit(TimeUnit::Invalid)
{
}

AgeSpec::AgeSpec(int age, TimeUnit unit)
    : age(age)
    , unit(unit)
{
}

QString AgeSpec::format(I18nContext context) const
{
    switch (unit) {
    case TimeUnit::Invalid:
        return {};
    case TimeUnit::Days:
        if (context == I18nContext::Birthday)
            return i18ncp("As in 'The baby is 1 day old'", "1 day", "%1 days", age);
        else
            return i18ncp("As in 'This happened 1 day ago'", "1 day ago", "%1 days ago", age);
    case TimeUnit::Months:
        if (context == I18nContext::Birthday)
            return i18ncp("As in 'The baby is 1 month old'", "1 month", "%1 months", age);
        else
            return i18ncp("As in 'This happened 1 month ago'", "1 month ago", "%1 months ago", age);
    case TimeUnit::Years:
        if (context == I18nContext::Birthday)
            return i18ncp("As in 'The baby is 1 year old'", "1 year", "%1 years", age);
        else
            return i18ncp("As in 'This happened 1 year ago'", "1 year ago", "%1 years ago", age);
    }
    Q_ASSERT(false);
    return {};
}

bool AgeSpec::isValid() const
{
    return unit != TimeUnit::Invalid;
}

bool AgeSpec::operator==(const AgeSpec &other) const
{
    return (age == other.age && unit == other.unit);
}

/**
 * @brief dateDifference computes the difference between two dates with an appropriate unit.
 * It can be used to generate human readable date differences,
 * e.g. "6 months" instead of "0.5 years".
 *
 * @param priorDate
 * @param laterDate
 * @return a DateSpec with appropriate scale.
 */
AgeSpec dateDifference(const QDate &priorDate, const QDate &laterDate)
{
    const int priorDay = priorDate.day();
    const int laterDay = laterDate.day();
    const int priorMonth = priorDate.month();
    const int laterMonth = laterDate.month();
    const int priorYear = priorDate.year();
    const int laterYear = laterDate.year();

    // Image before birth
    const int days = priorDate.daysTo(laterDate);
    if (days < 0)
        return {};

    if (days < 31)
        return { days, TimeUnit::Days };

    int months = (laterYear - priorYear) * 12;
    months += (laterMonth - priorMonth);
    months += (laterDay >= priorDay) ? 0 : -1;

    if (months < 24)
        return { months, TimeUnit::Months };
    else
        return { months / 12, TimeUnit::Years };
}

#ifdef TEST_DATEDIFF
void testDateDifference()
{
    using namespace Utilities;
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1971, 7, 11)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("0 days"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1971, 8, 10)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("30 days"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1971, 8, 11)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("1 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1971, 8, 12)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("1 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1971, 9, 10)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("1 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1971, 9, 11)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("2 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1972, 6, 10)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("10 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1972, 6, 11)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("11 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1972, 6, 12)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("11 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1972, 7, 10)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("11 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1972, 7, 11)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("12 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1972, 7, 12)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("12 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1972, 12, 11)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("17 month"));
    Q_ASSERT(dateDifference(QDate(1971, 7, 11), QDate(1973, 7, 11)).format(AgeSpec::I18nContext::Birthday) == QString::fromLatin1("2 years"));
    qDebug() << "Tested dateDifference without problems.";
}
#endif
}

QString Utilities::formatAge(DB::CategoryPtr category, const QString &item, DB::ImageInfoPtr info)
{
#ifdef TEST_DATEDIFF
    testDateDifference(); // I wish I could get my act together to set up a test suite.
#endif
    const QDate birthDate = category->birthDate(item);
    const QDate start = info->date().start().date();
    const QDate end = info->date().end().date();

    if (birthDate.isNull() || !info->date().isValid())
        return {};

    const AgeSpec minAge = dateDifference(birthDate, start);
    const AgeSpec maxAge = dateDifference(birthDate, end);
    if (minAge == maxAge)
        return i18n(" (%1)", minAge.format(AgeSpec::I18nContext::Birthday));
    else if (!minAge.isValid())
        return i18n(" (&lt; %1)", maxAge.format(AgeSpec::I18nContext::Birthday));
    else {
        if (minAge.unit == maxAge.unit)
            return i18nc("E.g. ' (1-2 years)'", " (%1-%2)", minAge.age, maxAge.format(AgeSpec::I18nContext::Birthday));
        else
            return i18nc("E.g. ' (7 months-1 year)'", " (%1-%2)", minAge.format(AgeSpec::I18nContext::Birthday), maxAge.format(AgeSpec::I18nContext::Birthday));
    }
}

QString Utilities::timeAgo(const DB::ImageInfoPtr info)
{
    const QDate startDate = info->date().start().date();
    const QDate endDate = info->date().end().date();
    const QDate today = QDate::currentDate();

    if (!info->date().isValid())
        return {};

    const AgeSpec minTimeAgo = dateDifference(endDate, today);
    const AgeSpec maxTimeAgo = dateDifference(startDate, today);
    if (!minTimeAgo.isValid()) {
        return {};
    }
    if (minTimeAgo == maxTimeAgo) {
        return i18n(" (%1)", minTimeAgo.format(AgeSpec::I18nContext::Anniversary));
    } else {
        if (minTimeAgo.unit == maxTimeAgo.unit)
            return i18nc("E.g. ' (1-2 years ago)'", " (%1-%2)", minTimeAgo.age, maxTimeAgo.format(AgeSpec::I18nContext::Anniversary));
        else
            return i18nc("E.g. '(7 months ago-1 year ago)'", " (%1-%2)", minTimeAgo.format(AgeSpec::I18nContext::Anniversary), maxTimeAgo.format(AgeSpec::I18nContext::Anniversary));
    }
}
