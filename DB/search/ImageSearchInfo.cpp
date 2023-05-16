// SPDX-FileCopyrightText: 2003-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2005-2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2006-2008 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007-2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2007-2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2009 Hassan Ibraheem <hasan.ibraheem@gmail.com>
// SPDX-FileCopyrightText: 2011-2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2012-2016, 2018-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2020 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2017-2020 Robert Krawitz <rlk@alum.mit.edu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageSearchInfo.h"

#include "AndCategoryMatcher.h"
#include "CategoryMatcher.h"
#include "ContainerCategoryMatcher.h"
#include "ExactCategoryMatcher.h"
#include "NegationCategoryMatcher.h"
#include "NoTagCategoryMatcher.h"
#include "OrCategoryMatcher.h"
#include "ValueCategoryMatcher.h"
#include "WildcardCategoryMatcher.h"

#include <DB/ImageDB.h>
#include <ImageManager/RawImageDecoder.h>
#include <kpabase/FileExtensions.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QApplication>
#include <QRegExp>
#include <QRegularExpression>

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
        && m_rating == -1 && m_megapixel == 0 && m_exifSearchInfo.isEmpty()
        && m_categoryMatchText.isEmpty()
        && freeformMatchText().isEmpty()
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

    // ok = ok && (_rating == -1 ) || ( _rating == info->rating() );
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
    Utilities::FastDateTime actualStart = info->date().start();
    Utilities::FastDateTime actualEnd = info->date().end();

    if (m_date.start().isValid()) {
        if (actualEnd < m_date.start() || (m_date.end().isValid() && actualStart > m_date.end()))
            return false;
    } else if (m_date.end().isValid() && actualStart > m_date.end()) {
        return false;
    }

    // -------------------------------------------------- Label
    if (!m_label.isEmpty() && info->label().indexOf(m_label) == -1)
        return false;

    // -------------------------------------------------- RAW
    if (m_searchRAW && !KPABase::isUsableRawImage(info->fileName()))
        return false;

#ifdef HAVE_MARBLE
    // Search for GPS Position
    if (!m_regionSelection.isNull()) {
        if (!info->coordinates().hasCoordinates())
            return false;

        if (!m_regionSelection.contains(info->coordinates()))
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
        const QStringList list = m_description.split(QChar::fromLatin1(' '), Qt::SkipEmptyParts);
        for (const QString &word : list) {
            if (txt.indexOf(word, 0, Qt::CaseInsensitive) == -1)
                return false;
        }
    }

    // -------------------------------------------------- EXIF
    if (!m_exifSearchInfo.matches(info->fileName()))
        return false;

    // -------------------------------------------------- Freeform
    if (!freeformMatchText().isEmpty()) {
        const auto re = m_freeformMatcher.regularExpression();
        if (!re.match(info->label()).hasMatch()
            && !re.match(info->fileName().relative()).hasMatch()
            && !re.match(info->description()).hasMatch()
            && !m_freeformMatcher.eval(info)) {
            return false;
        }
    }

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
    m_matchGeneration = nextGeneration();
    // compiled data is not affected
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

QVariantMap ImageSearchInfo::getLockData() const
{
    QVariantMap pairs;
    pairs[QString::fromLatin1("label")] = m_label;
    pairs[QString::fromLatin1("description")] = m_description;
    pairs[QString::fromLatin1("categories")] = QVariant(m_categoryMatchText.keys());
    for (QMap<QString, QString>::ConstIterator it = m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it) {
        pairs[it.key()] = it.value();
    }
    return pairs;
}

ImageSearchInfo ImageSearchInfo::loadLock(const QMap<QString, QVariant> &keyValuePairs)
{
    ImageSearchInfo info;
    info.m_label = keyValuePairs.value(QString::fromLatin1("label"), {}).toString();
    info.m_description = keyValuePairs.value(QString::fromLatin1("description"), {}).toString();
    QStringList categories = keyValuePairs.value(QString::fromLatin1("categories"), {}).toStringList();
    for (QStringList::ConstIterator it = categories.constBegin(); it != categories.constEnd(); ++it) {
        info.setCategoryMatchText(*it, keyValuePairs.value(*it, {}).toString());
    }
    return info;
}

void ImageSearchInfo::compile() const
{
    qCDebug(DBCategoryMatcherLog) << "Compiling search info...";
    m_exifSearchInfo.search();

    CompiledDataPrivate compiledData;

    for (QMap<QString, QString>::ConstIterator it = m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it) {
        const QString category = it.key();
        const QString matchText = it.value();

        const QStringList orParts = matchText.split(QString::fromLatin1("|"), Qt::SkipEmptyParts);
        DB::ContainerCategoryMatcher *orMatcher = new DB::OrCategoryMatcher;

        for (QString orPart : orParts) {
            // Split by " & ", not only by "&", so that the doubled "&"s won't be used as a split point
            const QStringList andParts = orPart.split(QString::fromLatin1(" & "), Qt::SkipEmptyParts);

            DB::ContainerCategoryMatcher *andMatcher;
            bool exactMatch = false;
            bool negate = false;
            andMatcher = new DB::AndCategoryMatcher;

            for (QString str : andParts) {
                static const QRegExp regexp(QString::fromLatin1("^\\s*!\\s*(.*)$"));
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
                    if (negate) {
                        valueMatcher = new DB::NegationCategoryMatcher(valueMatcher);
                        negate = false;
                    }
                }
                andMatcher->addElement(valueMatcher);
            }
            if (exactMatch) {
                DB::CategoryMatcher *exactMatcher = nullptr;
                // if andMatcher has exactMatch set, but no CategoryMatchers, then
                // matching "category / None" is what we want:
                if (andMatcher->mp_elements.count() == 0) {
                    delete andMatcher;
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
            else
                delete andMatcher;
        }
        CategoryMatcher *matcher = nullptr;
        if (orMatcher->mp_elements.count() == 1)
            matcher = orMatcher->mp_elements[0];
        else if (orMatcher->mp_elements.count() > 1)
            matcher = orMatcher;
        else
            delete orMatcher;

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
    const QString str = categoryMatchText(group);
    if (str.contains(QString::fromLatin1("|"))) {
        return result;
    }

    const QStringList list = str.split(QString::fromLatin1("&"), Qt::SkipEmptyParts);
    for (const QString &part : list) {
        const QString nm = part.trimmed();
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
    OrCategoryMatcher *orMatcher;

    if ((orMatcher = dynamic_cast<OrCategoryMatcher *>(item))) {
        for (CategoryMatcher *child : orMatcher->mp_elements) {
            result.append(extractAndMatcher(child));
        }
    } else
        result.append(extractAndMatcher(item));
    return result;
}

QString ImageSearchInfo::freeformMatchText() const
{
    return m_freeformMatcher.regularExpression().pattern();
}

void ImageSearchInfo::setFreeformMatchText(const QString &freeformMatchText)
{
    setCacheable(false);
    QRegularExpression re { freeformMatchText, QRegularExpression::CaseInsensitiveOption };
    m_freeformMatcher.setRegularExpression(re);
    m_isNull = m_isNull && freeformMatchText.isEmpty();
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

#ifdef HAVE_MARBLE
Map::GeoCoordinates::LatLonBox ImageSearchInfo::regionSelection() const
{
    return m_regionSelection;
}

void ImageSearchInfo::setRegionSelection(const Map::GeoCoordinates::LatLonBox &actRegionSelection)
{
    m_regionSelection = actRegionSelection;
    if (!m_regionSelection.isNull()) {
        m_isNull = false;
    }
    m_matchGeneration = nextGeneration();
    // compiled data is not affected
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
