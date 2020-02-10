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

#include "ImageSearchInfo.h"

#include "AndCategoryMatcher.h"
#include "CategoryMatcher.h"
#include "ContainerCategoryMatcher.h"
#include "ExactCategoryMatcher.h"
#include "ImageDB.h"
#include "Logging.h"
#include "NegationCategoryMatcher.h"
#include "NoTagCategoryMatcher.h"
#include "OrCategoryMatcher.h"
#include "ValueCategoryMatcher.h"

#include <ImageManager/RawImageDecoder.h>
#include <Settings/SettingsData.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QApplication>
#include <QRegExp>

using namespace DB;

static QAtomicInt s_matchGeneration;
static int nextGeneration()
{
    return s_matchGeneration++;
}

ImageSearchInfo::ImageSearchInfo()
    : m_matchGeneration(nextGeneration())
{
}

ImageSearchInfo::ImageSearchInfo(const ImageDate &date,
                                 const QString &label, const QString &description)
    : m_date(date)
    , m_label(label)
    , m_description(description)
    , m_isNull(false)
    , m_matchGeneration(nextGeneration())
{
}

ImageSearchInfo::ImageSearchInfo(const ImageDate &date,
                                 const QString &label, const QString &description,
                                 const QString &fnPattern)
    : m_date(date)
    , m_label(label)
    , m_description(description)
    , m_fnPattern(fnPattern)
    , m_isNull(false)
    , m_matchGeneration(nextGeneration())
{
}

QString ImageSearchInfo::label() const
{
    return m_label;
}

QRegExp ImageSearchInfo::fnPattern() const
{
    return m_fnPattern;
}

QString ImageSearchInfo::description() const
{
    return m_description;
}

void ImageSearchInfo::checkIfNull()
{
    if (m_compiled.valid || isNull())
        return;
    if (m_date.isNull() && m_label.isEmpty() && m_description.isEmpty()
        && m_rating == -1 && m_megapixel == 0 && m_exifSearchInfo.isNull()
        && m_categoryMatchText.isEmpty()
#ifdef HAVE_KGEOMAP
        && !m_regionSelection.first.hasCoordinates() && !m_regionSelection.second.hasCoordinates()
#endif
    ) {
        m_isNull = true;
    }
}

bool ImageSearchInfo::isNull() const
{
    return m_isNull;
}

bool ImageSearchInfo::isCacheable() const
{
    return m_isCacheable;
}

void ImageSearchInfo::setCacheable(bool cacheable)
{
    m_isCacheable = cacheable;
}

bool ImageSearchInfo::match(ImageInfoPtr info) const
{
    if (m_isNull)
        return true;

    if (m_isCacheable && info->matchGeneration() == m_matchGeneration)
        return info->isMatched();

    bool ok = doMatch(info);

    if (m_isCacheable) {
        info->setMatchGeneration(m_matchGeneration);
        info->setIsMatched(ok);
    }
    return ok;
}

bool ImageSearchInfo::doMatch(ImageInfoPtr info) const
{
    if (!m_compiled.valid)
        compile();

    // -------------------------------------------------- Rating

    //ok = ok && (_rating == -1 ) || ( _rating == info->rating() );
    if (m_rating != -1) {
        switch (m_ratingSearchMode) {
        case 1:
            // Image rating at least selected
            if (m_rating > info->rating())
                return false;
            break;
        case 2:
            // Image rating less than selected
            if (m_rating < info->rating())
                return false;
            break;
        case 3:
            // Image rating not equal
            if (m_rating == info->rating())
                return false;
            break;
        default:
            if (m_rating != info->rating())
                return false;
            break;
        }
    }

    // -------------------------------------------------- Resolution
    if (m_megapixel && (m_megapixel * 1000000 > info->size().width() * info->size().height()))
        return false;

    if (m_max_megapixel && m_max_megapixel < m_megapixel && (m_max_megapixel * 1000000 < info->size().width() * info->size().height()))
        return false;

    // -------------------------------------------------- Date
    QDateTime actualStart = info->date().start();
    QDateTime actualEnd = info->date().end();

    if (m_date.start().isValid()) {
        if (actualEnd < m_date.start() || (m_date.end().isValid() && actualStart > m_date.end()))
            return false;
    } else if (m_date.end().isValid() && actualStart > m_date.end()) {
        return false;
    }

    // -------------------------------------------------- Label
    if (m_label.isEmpty() && info->label().indexOf(m_label) == -1)
        return false;

    // -------------------------------------------------- RAW
    if (m_searchRAW && !ImageManager::RAWImageDecoder::isRAW(info->fileName()))
        return false;

#ifdef HAVE_KGEOMAP
    // Search for GPS Position
    if (m_usingRegionSelection) {
        if (!info->coordinates().hasCoordinates())
            return false;
        float infoLat = info->coordinates().lat();
        if (m_regionSelectionMinLat > infoLat || m_regionSelectionMaxLat < infoLat)
            return false;
        float infoLon = info->coordinates().lon();
        if (m_regionSelectionMinLon > infoLon || m_regionSelectionMaxLon < infoLon)
            return false;
    }
#endif

    // -------------------------------------------------- File name pattern
    if (!m_fnPattern.isEmpty() && m_fnPattern.indexIn(info->fileName().relative()) == -1)
        return false;

    // -------------------------------------------------- Options
    // alreadyMatched map is used to make it possible to search for
    // Jesper & None
    QMap<QString, StringSet> alreadyMatched;
    for (CategoryMatcher *optionMatcher : m_compiled.categoryMatchers) {
        if (!optionMatcher->eval(info, alreadyMatched))
            return false;
    }

    // -------------------------------------------------- Text
    if (!m_description.isEmpty()) {
        const QString &txt(info->description());
        QStringList list = m_description.split(QChar::fromLatin1(' '), QString::SkipEmptyParts);
        Q_FOREACH (const QString &word, list) {
            if (txt.indexOf(word, 0, Qt::CaseInsensitive) == -1)
                return false;
        }
    }

    // -------------------------------------------------- EXIF
    if (!m_exifSearchInfo.matches(info->fileName()))
        return false;

    return true;
}

QString ImageSearchInfo::categoryMatchText(const QString &name) const
{
    return m_categoryMatchText[name];
}

void ImageSearchInfo::setCategoryMatchText(const QString &name, const QString &value)
{
    if (value.isEmpty()) {
        m_categoryMatchText.remove(name);
    } else {
        m_categoryMatchText[name] = value;
    }
    m_isNull = false;
    m_compiled.valid = false;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::addAnd(const QString &category, const QString &value)
{
    // Escape literal "&"s in value by doubling it
    QString escapedValue = value;
    escapedValue.replace(QString::fromUtf8("&"), QString::fromUtf8("&&"));

    QString val = categoryMatchText(category);
    if (!val.isEmpty())
        val += QString::fromLatin1(" & ") + escapedValue;
    else
        val = escapedValue;

    setCategoryMatchText(category, val);
    m_isNull = false;
    m_compiled.valid = false;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setRating(short rating)
{
    m_rating = rating;
    m_isNull = false;
    m_compiled.valid = false;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setMegaPixel(short megapixel)
{
    m_megapixel = megapixel;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setMaxMegaPixel(short max_megapixel)
{
    m_max_megapixel = max_megapixel;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setSearchMode(int index)
{
    m_ratingSearchMode = index;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setSearchRAW(bool searchRAW)
{
    m_searchRAW = searchRAW;
    m_matchGeneration = nextGeneration();
}

QString ImageSearchInfo::toString() const
{
    QString res;
    bool first = true;
    for (QMap<QString, QString>::ConstIterator it = m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it) {
        if (!it.value().isEmpty()) {
            if (first)
                first = false;
            else
                res += QString::fromLatin1(" / ");

            QString txt = it.value();
            if (txt == ImageDB::NONE())
                txt = i18nc("As in No persons, no locations etc. I do realize that translators may have problem with this, "
                            "but I need some how to indicate the category, and users may create their own categories, so this is "
                            "the best I can do - Jesper.",
                            "No %1", it.key());

            if (txt.contains(QString::fromLatin1("|")))
                txt.replace(QString::fromLatin1("&"), QString::fromLatin1(" %1 ").arg(i18n("and")));

            else
                txt.replace(QString::fromLatin1("&"), QString::fromLatin1(" / "));

            txt.replace(QString::fromLatin1("|"), QString::fromLatin1(" %1 ").arg(i18n("or")));
            txt.replace(QString::fromLatin1("!"), QString::fromLatin1(" %1 ").arg(i18n("not")));
            txt.replace(ImageDB::NONE(), i18nc("As in no other persons, or no other locations. "
                                               "I do realize that translators may have problem with this, "
                                               "but I need some how to indicate the category, and users may create their own categories, so this is "
                                               "the best I can do - Jesper.",
                                               "No other %1", it.key()));

            res += txt.simplified();
        }
    }
    return res;
}

void ImageSearchInfo::debug()
{
    for (QMap<QString, QString>::Iterator it = m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it) {
        qCDebug(DBCategoryMatcherLog) << it.key() << ", " << it.value();
    }
}

// PENDING(blackie) move this into the Options class instead of having it here.
void ImageSearchInfo::saveLock() const
{
    KConfigGroup config = KSharedConfig::openConfig()->group(Settings::SettingsData::instance()->groupForDatabase("Privacy Settings"));
    config.writeEntry(QString::fromLatin1("label"), m_label);
    config.writeEntry(QString::fromLatin1("description"), m_description);
    config.writeEntry(QString::fromLatin1("categories"), m_categoryMatchText.keys());
    for (QMap<QString, QString>::ConstIterator it = m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it) {
        config.writeEntry(it.key(), it.value());
    }
    config.sync();
}

ImageSearchInfo ImageSearchInfo::loadLock()
{
    KConfigGroup config = KSharedConfig::openConfig()->group(Settings::SettingsData::instance()->groupForDatabase("Privacy Settings"));
    ImageSearchInfo info;
    info.m_label = config.readEntry("label");
    info.m_description = config.readEntry("description");
    QStringList categories = config.readEntry<QStringList>(QString::fromLatin1("categories"), QStringList());
    for (QStringList::ConstIterator it = categories.constBegin(); it != categories.constEnd(); ++it) {
        info.setCategoryMatchText(*it, config.readEntry<QString>(*it, QString()));
    }
    return info;
}

void ImageSearchInfo::compile() const
{
    m_exifSearchInfo.search();
#ifdef HAVE_KGEOMAP
    // Prepare Search for GPS Position
    m_usingRegionSelection = m_regionSelection.first.hasCoordinates() && m_regionSelection.second.hasCoordinates();
    if (m_usingRegionSelection) {
        using std::max;
        using std::min;
        m_regionSelectionMinLat = min(m_regionSelection.first.lat(), m_regionSelection.second.lat());
        m_regionSelectionMaxLat = max(m_regionSelection.first.lat(), m_regionSelection.second.lat());
        m_regionSelectionMinLon = min(m_regionSelection.first.lon(), m_regionSelection.second.lon());
        m_regionSelectionMaxLon = max(m_regionSelection.first.lon(), m_regionSelection.second.lon());
    }
#endif

    CompiledDataPrivate compiledData;

    for (QMap<QString, QString>::ConstIterator it = m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it) {
        QString category = it.key();
        QString matchText = it.value();

        QStringList orParts = matchText.split(QString::fromLatin1("|"), QString::SkipEmptyParts);
        DB::ContainerCategoryMatcher *orMatcher = new DB::OrCategoryMatcher;

        Q_FOREACH (QString orPart, orParts) {
            // Split by " & ", not only by "&", so that the doubled "&"s won't be used as a split point
            QStringList andParts = orPart.split(QString::fromLatin1(" & "), QString::SkipEmptyParts);

            DB::ContainerCategoryMatcher *andMatcher;
            bool exactMatch = false;
            bool negate = false;
            andMatcher = new DB::AndCategoryMatcher;

            Q_FOREACH (QString str, andParts) {
                static QRegExp regexp(QString::fromLatin1("^\\s*!\\s*(.*)$"));
                if (regexp.exactMatch(str)) { // str is preceded with NOT
                    negate = true;
                    str = regexp.cap(1);
                }
                str = str.trimmed();
                CategoryMatcher *valueMatcher;
                if (str == ImageDB::NONE()) { // mark AND-group as containing a "No other" condition
                    exactMatch = true;
                    continue;
                } else {
                    valueMatcher = new DB::ValueCategoryMatcher(category, str);
                    if (negate)
                        valueMatcher = new DB::NegationCategoryMatcher(valueMatcher);
                }
                andMatcher->addElement(valueMatcher);
            }
            if (exactMatch) {
                DB::CategoryMatcher *exactMatcher = nullptr;
                // if andMatcher has exactMatch set, but no CategoryMatchers, then
                // matching "category / None" is what we want:
                if (andMatcher->mp_elements.count() == 0) {
                    exactMatcher = new DB::NoTagCategoryMatcher(category);
                } else {
                    ExactCategoryMatcher *noOtherMatcher = new ExactCategoryMatcher(category);
                    if (andMatcher->mp_elements.count() == 1)
                        noOtherMatcher->setMatcher(andMatcher->mp_elements[0]);
                    else
                        noOtherMatcher->setMatcher(andMatcher);
                    exactMatcher = noOtherMatcher;
                }
                if (negate)
                    exactMatcher = new DB::NegationCategoryMatcher(exactMatcher);
                orMatcher->addElement(exactMatcher);
            } else if (andMatcher->mp_elements.count() == 1)
                orMatcher->addElement(andMatcher->mp_elements[0]);
            else if (andMatcher->mp_elements.count() > 1)
                orMatcher->addElement(andMatcher);
        }
        CategoryMatcher *matcher = nullptr;
        if (orMatcher->mp_elements.count() == 1)
            matcher = orMatcher->mp_elements[0];
        else if (orMatcher->mp_elements.count() > 1)
            matcher = orMatcher;

        if (matcher) {
            compiledData.categoryMatchers.append(matcher);
            if (DBCategoryMatcherLog().isDebugEnabled()) {
                qCDebug(DBCategoryMatcherLog) << "Matching text '" << matchText << "' in category " << category << ":";
                matcher->debug(0);
                qCDebug(DBCategoryMatcherLog) << ".";
            }
        }
    }
    compiledData.valid = true;
    std::swap(m_compiled, compiledData);
}

void ImageSearchInfo::debugMatcher() const
{
    if (!m_compiled.valid)
        compile();

    qCDebug(DBCategoryMatcherLog, "And:");
    for (CategoryMatcher *optionMatcher : m_compiled.categoryMatchers) {
        optionMatcher->debug(1);
    }
}

QList<QList<SimpleCategoryMatcher *>> ImageSearchInfo::query() const
{
    if (!m_compiled.valid)
        compile();

    // Combine _optionMachers to one list of lists in Disjunctive
    // Normal Form and return it.

    QList<CategoryMatcher *>::Iterator it = m_compiled.categoryMatchers.begin();
    QList<QList<SimpleCategoryMatcher *>> result;
    if (it == m_compiled.categoryMatchers.end())
        return result;

    result = convertMatcher(*it);
    ++it;

    for (; it != m_compiled.categoryMatchers.end(); ++it) {
        QList<QList<SimpleCategoryMatcher *>> current = convertMatcher(*it);
        QList<QList<SimpleCategoryMatcher *>> oldResult = result;
        result.clear();

        for (QList<SimpleCategoryMatcher *> resultIt : oldResult) {
            for (QList<SimpleCategoryMatcher *> currentIt : current) {
                QList<SimpleCategoryMatcher *> tmp;
                tmp += resultIt;
                tmp += currentIt;
                result.append(tmp);
            }
        }
    }
    return result;
}

Utilities::StringSet ImageSearchInfo::findAlreadyMatched(const QString &group) const
{
    Utilities::StringSet result;
    QString str = categoryMatchText(group);
    if (str.contains(QString::fromLatin1("|"))) {
        return result;
    }

    QStringList list = str.split(QString::fromLatin1("&"), QString::SkipEmptyParts);
    Q_FOREACH (QString part, list) {
        QString nm = part.trimmed();
        if (!nm.contains(QString::fromLatin1("!")))
            result.insert(nm);
    }
    return result;
}

QList<SimpleCategoryMatcher *> ImageSearchInfo::extractAndMatcher(CategoryMatcher *matcher) const
{
    QList<SimpleCategoryMatcher *> result;

    AndCategoryMatcher *andMatcher;
    SimpleCategoryMatcher *simpleMatcher;

    if ((andMatcher = dynamic_cast<AndCategoryMatcher *>(matcher))) {
        for (CategoryMatcher *child : andMatcher->mp_elements) {
            SimpleCategoryMatcher *simpleMatcher = dynamic_cast<SimpleCategoryMatcher *>(child);
            Q_ASSERT(simpleMatcher);
            result.append(simpleMatcher);
        }
    } else if ((simpleMatcher = dynamic_cast<SimpleCategoryMatcher *>(matcher)))
        result.append(simpleMatcher);
    else
        Q_ASSERT(false);

    return result;
}

/** Convert matcher to Disjunctive Normal Form.
 *
 * @return OR-list of AND-lists. (e.g. OR(AND(a,b),AND(c,d)))
 */
QList<QList<SimpleCategoryMatcher *>> ImageSearchInfo::convertMatcher(CategoryMatcher *item) const
{
    QList<QList<SimpleCategoryMatcher *>> result;
    OrCategoryMatcher *orMacther;

    if ((orMacther = dynamic_cast<OrCategoryMatcher *>(item))) {
        for (CategoryMatcher *child : orMacther->mp_elements) {
            result.append(extractAndMatcher(child));
        }
    } else
        result.append(extractAndMatcher(item));
    return result;
}

short ImageSearchInfo::rating() const
{
    return m_rating;
}

ImageDate ImageSearchInfo::date() const
{
    return m_date;
}

void ImageSearchInfo::addExifSearchInfo(const Exif::SearchInfo info)
{
    m_exifSearchInfo = info;
    m_isNull = false;
    m_matchGeneration = nextGeneration();
}

void DB::ImageSearchInfo::renameCategory(const QString &oldName, const QString &newName)
{
    m_categoryMatchText[newName] = m_categoryMatchText[oldName];
    m_categoryMatchText.remove(oldName);
    m_compiled.valid = false;
    m_matchGeneration = nextGeneration();
}

#ifdef HAVE_KGEOMAP
KGeoMap::GeoCoordinates::Pair ImageSearchInfo::regionSelection() const
{
    return m_regionSelection;
}

void ImageSearchInfo::setRegionSelection(const KGeoMap::GeoCoordinates::Pair &actRegionSelection)
{
    m_regionSelection = actRegionSelection;
    m_compiled.valid = false;
    if (m_regionSelection.first.hasCoordinates() && m_regionSelection.second.hasCoordinates()) {
        m_isNull = false;
    }
    m_matchGeneration = nextGeneration();
}
#endif

ImageSearchInfo::CompiledDataPrivate::CompiledDataPrivate(const ImageSearchInfo::CompiledDataPrivate &)
{
    // copying invalidates the compiled data
    valid = false;
}

ImageSearchInfo::CompiledDataPrivate::~CompiledDataPrivate()
{
    qDeleteAll(categoryMatchers);
    categoryMatchers.clear();
}

ImageSearchInfo::CompiledDataPrivate &ImageSearchInfo::CompiledDataPrivate::operator=(const ImageSearchInfo::CompiledDataPrivate &)
{
    // copying invalidates the compiled data
    valid = false;
    return *this;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
