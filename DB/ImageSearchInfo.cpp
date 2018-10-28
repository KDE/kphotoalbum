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

#include <QRegExp>
#include <QApplication>

using namespace DB;

static QAtomicInt s_matchGeneration;
static int nextGeneration()
{
    return s_matchGeneration++;
}

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description )
    : m_date( date), m_label( label ), m_description( description ), m_rating( -1 ), m_megapixel( 0 ), m_max_megapixel( 0 ), m_ratingSearchMode( 0 ), m_searchRAW( false ), m_isNull( false ), m_isCacheable( true ), m_compiled( false ), m_matchGeneration(nextGeneration())
{
}

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description,
                  const QString& fnPattern )
    : m_date( date), m_label( label ), m_description( description ), m_fnPattern( fnPattern ), m_rating( -1 ), m_megapixel( 0 ), m_max_megapixel( 0 ), m_ratingSearchMode( 0 ), m_searchRAW( false ), m_isNull( false ), m_isCacheable( true ), m_compiled( false ), m_matchGeneration(nextGeneration())
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

ImageSearchInfo::ImageSearchInfo()
    : m_rating( -1 ), m_megapixel( 0 ), m_max_megapixel( 0 ), m_ratingSearchMode( 0 ), m_searchRAW( false ), m_isNull( true ), m_isCacheable( true ), m_compiled( false ), m_matchGeneration(nextGeneration())
{
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

bool ImageSearchInfo::match( ImageInfoPtr info ) const
{
    if ( m_isNull )
        return true;

    if (  m_isCacheable && info->matchGeneration() == m_matchGeneration )
        return info->isMatched();

    bool ok = doMatch( info );

    if ( m_isCacheable ) {
        info->setMatchGeneration(m_matchGeneration);
        info->setIsMatched(ok);
    }
    return ok;
}

bool ImageSearchInfo::doMatch( ImageInfoPtr info ) const
{
    if ( !m_compiled )
        compile();

    if ( ! m_exifSearchInfo.matches( info->fileName() ) )
        return false;

    QDateTime actualStart = info->date().start();
    QDateTime actualEnd = info->date().end();
    if ( actualEnd <= actualStart )  {
        QDateTime tmp = actualStart;
        actualStart = actualEnd;
        actualEnd = tmp;
    }

    if ( !m_date.start().isNull() ) {
        // Date
        // the search date matches the actual date if:
        // actual.start <= search.start <= actual.end or
        // actual.start <= search.end <= actual.end or
        // search.start <= actual.start and actual.end <= search.end

        if ( actualEnd < m_date.start() ||
             ( !m_date.end().isNull() && actualStart > m_date.end() ) )
            return false;
    } else if ( !m_date.end().isNull() && actualStart > m_date.end() ) {
        return false;
    }

    // -------------------------------------------------- Options
    // alreadyMatched map is used to make it possible to search for
    // Jesper & None
    QMap<QString, StringSet> alreadyMatched;
    for (CategoryMatcher* optionMatcher : m_categoryMatchers) {
        if ( ! optionMatcher->eval(info, alreadyMatched) )
            return false;
    }

    // -------------------------------------------------- Label
    if ( m_label.isEmpty() && info->label().indexOf(m_label) == -1 )
        return false;

    // -------------------------------------------------- RAW
    if ( m_searchRAW && !ImageManager::RAWImageDecoder::isRAW( info->fileName()) )
        return false;

    // -------------------------------------------------- Rating

    //ok = ok && (_rating == -1 ) || ( _rating == info->rating() );
    if (m_rating != -1) {
        switch( m_ratingSearchMode ) {
        case 1:
            // Image rating at least selected
            if ( m_rating > info->rating() )
                return false;
            break;
        case 2:
            // Image rating less than selected
            if ( m_rating < info->rating() )
                return false;
            break;
        case 3:
            // Image rating not equal
            if ( m_rating == info->rating() )
                return false;
            break;
        default:
            if ( m_rating != info->rating() )
                return false;
            break;
        }
    }


    // -------------------------------------------------- Resolution
    if ( m_megapixel && 
         ( m_megapixel * 1000000 > info->size().width() * info->size().height() ) )
        return false;

    if ( m_max_megapixel && m_max_megapixel < m_megapixel &&
         ( m_max_megapixel * 1000000 < info->size().width() * info->size().height() ) )
        return false;

    // -------------------------------------------------- Text
    if ( !m_description.isEmpty() ) {
        const QString &txt(info->description());
        QStringList list = m_description.split(QChar::fromLatin1(' '), QString::SkipEmptyParts);
        Q_FOREACH( const QString &word, list ) {
            if ( txt.indexOf( word, 0, Qt::CaseInsensitive ) == -1 )
                return false;
        }
    }

    // -------------------------------------------------- File name pattern
    if ( !m_fnPattern.isEmpty() &&
         m_fnPattern.indexIn( info->fileName().relative() ) == -1 )
        return false;


#ifdef HAVE_KGEOMAP
    // Search for GPS Position
    if (m_usingRegionSelection) {
        if ( !info->coordinates().hasCoordinates() )
            return false;
        float infoLat = info->coordinates().lat();
        if ( m_regionSelectionMinLat > infoLat || 
             m_regionSelectionMaxLat < infoLat )
            return false;
        float infoLon = info->coordinates().lon();
        if ( m_regionSelectionMinLon > infoLon ||
             m_regionSelectionMaxLon < infoLon )
            return false;
    }
#endif

    return true;
}


QString ImageSearchInfo::categoryMatchText( const QString& name ) const
{
    return m_categoryMatchText[name];
}

void ImageSearchInfo::setCategoryMatchText( const QString& name, const QString& value )
{
    m_categoryMatchText[name] = value;
    m_isNull = false;
    m_compiled = false;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::addAnd( const QString& category, const QString& value )
{
    // Escape literal "&"s in value by doubling it
    QString escapedValue = value;
    escapedValue.replace(QString::fromUtf8("&"), QString::fromUtf8("&&"));

    QString val = categoryMatchText( category );
    if ( !val.isEmpty() )
        val += QString::fromLatin1( " & " ) + escapedValue;
    else
        val = escapedValue;

    setCategoryMatchText( category, val );
    m_isNull = false;
    m_compiled = false;
    m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setRating( short rating )
{
  m_rating = rating;
  m_isNull = false;
  m_compiled = false;
  m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setMegaPixel( short megapixel )
{
  m_megapixel = megapixel;
  m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setMaxMegaPixel( short max_megapixel )
{
  m_max_megapixel = max_megapixel;
  m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setSearchMode(int index)
{
  m_ratingSearchMode = index;
  m_matchGeneration = nextGeneration();
}

void ImageSearchInfo::setSearchRAW( bool searchRAW )
{
  m_searchRAW = searchRAW;
  m_matchGeneration = nextGeneration();
}


QString ImageSearchInfo::toString() const
{
    QString res;
    bool first = true;
    for( QMap<QString,QString>::ConstIterator it= m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it ) {
        if ( ! it.value().isEmpty() ) {
            if ( first )
                first = false;
            else
                res += QString::fromLatin1( " / " );

            QString txt = it.value();
            if ( txt == ImageDB::NONE() )
                txt = i18nc( "As in No persons, no locations etc. I do realize that translators may have problem with this, "
                            "but I need some how to indicate the category, and users may create their own categories, so this is "
                            "the best I can do - Jesper.", "No %1", it.key() );

            if ( txt.contains( QString::fromLatin1("|") ) )
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " %1 " ).arg( i18n("and") ) );

            else
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " / " ) );

            txt.replace( QString::fromLatin1( "|" ), QString::fromLatin1( " %1 " ).arg( i18n("or") ) );
            txt.replace( QString::fromLatin1( "!" ), QString::fromLatin1( " %1 " ).arg( i18n("not") ) );
            txt.replace( ImageDB::NONE(), i18nc( "As in no other persons, or no other locations. "
                                                "I do realize that translators may have problem with this, "
                                                "but I need some how to indicate the category, and users may create their own categories, so this is "
                                                "the best I can do - Jesper.", "No other %1", it.key() ) );

            res += txt.simplified();
        }
    }
    return res;
}

void ImageSearchInfo::debug()
{
    for( QMap<QString,QString>::Iterator it= m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it ) {
        qCDebug(DBCategoryMatcherLog) << it.key() << ", " << it.value();
    }
}

// PENDING(blackie) move this into the Options class instead of having it here.
void ImageSearchInfo::saveLock() const
{
    KConfigGroup config = KSharedConfig::openConfig()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings"));
    config.writeEntry( QString::fromLatin1("label"), m_label );
    config.writeEntry( QString::fromLatin1("description"), m_description );
    config.writeEntry( QString::fromLatin1("categories"), m_categoryMatchText.keys() );
    for( QMap<QString,QString>::ConstIterator it= m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it ) {
        config.writeEntry( it.key(), it.value() );
    }
    config.sync();
}

ImageSearchInfo ImageSearchInfo::loadLock()
{
    KConfigGroup config = KSharedConfig::openConfig()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings" ));
    ImageSearchInfo info;
    info.m_label = config.readEntry( "label" );
    info.m_description = config.readEntry( "description" );
    QStringList categories = config.readEntry<QStringList>( QString::fromLatin1("categories"), QStringList() );
    for( QStringList::ConstIterator it = categories.constBegin(); it != categories.constEnd(); ++it ) {
        info.setCategoryMatchText( *it, config.readEntry<QString>( *it, QString() ) );
    }
    return info;
}

ImageSearchInfo::ImageSearchInfo( const ImageSearchInfo& other )
{
    m_date = other.m_date;
    m_categoryMatchText = other.m_categoryMatchText;
    m_label = other.m_label;
    m_description = other.m_description;
    m_fnPattern = other.m_fnPattern;
    m_isNull = other.m_isNull;
    m_compiled = false;
    m_rating = other.m_rating;
    m_ratingSearchMode = other.m_ratingSearchMode;
    m_megapixel = other.m_megapixel;
    m_max_megapixel = other.m_max_megapixel;
    m_searchRAW = other.m_searchRAW;
    m_exifSearchInfo = other.m_exifSearchInfo;
    m_matchGeneration = other.m_matchGeneration;
    m_isCacheable = other.m_isCacheable;
#ifdef HAVE_KGEOMAP
    m_regionSelection = other.m_regionSelection;
#endif

}

void ImageSearchInfo::compile() const
{
    m_exifSearchInfo.search();
#ifdef HAVE_KGEOMAP
    // Prepare Search for GPS Position
    m_usingRegionSelection = m_regionSelection.first.hasCoordinates() && m_regionSelection.second.hasCoordinates();
    if (m_usingRegionSelection) {
        using std::min;
        using std::max;
        m_regionSelectionMinLat = min(m_regionSelection.first.lat(), m_regionSelection.second.lat());
        m_regionSelectionMaxLat = max(m_regionSelection.first.lat(), m_regionSelection.second.lat());
        m_regionSelectionMinLon = min(m_regionSelection.first.lon(), m_regionSelection.second.lon());
        m_regionSelectionMaxLon = max(m_regionSelection.first.lon(), m_regionSelection.second.lon());
    }
#endif

    deleteMatchers();

    for( QMap<QString,QString>::ConstIterator it = m_categoryMatchText.begin(); it != m_categoryMatchText.end(); ++it ) {
        QString category = it.key();
        QString matchText = it.value();

        QStringList orParts = matchText.split(QString::fromLatin1("|"), QString::SkipEmptyParts);
        DB::ContainerCategoryMatcher* orMatcher = new DB::OrCategoryMatcher;

        Q_FOREACH( QString orPart, orParts ) {
            // Split by " & ", not only by "&", so that the doubled "&"s won't be used as a split point
            QStringList andParts = orPart.split(QString::fromLatin1(" & "), QString::SkipEmptyParts);

            DB::ContainerCategoryMatcher* andMatcher;
            bool exactMatch=false;
            bool negate = false;
            andMatcher = new DB::AndCategoryMatcher;

            Q_FOREACH( QString str, andParts ) {
                static QRegExp regexp( QString::fromLatin1("^\\s*!\\s*(.*)$") );
                if ( regexp.exactMatch( str ) )
                { // str is preceded with NOT
                    negate = true;
                    str = regexp.cap(1);
                }
                str = str.trimmed();
                CategoryMatcher* valueMatcher;
                if ( str == ImageDB::NONE() )
                { // mark AND-group as containing a "No other" condition
                    exactMatch = true;
                    continue;
                }
                else
                {
                    valueMatcher = new DB::ValueCategoryMatcher( category, str );
                    if ( negate )
                        valueMatcher = new DB::NegationCategoryMatcher( valueMatcher );
                }
                andMatcher->addElement( valueMatcher );
            }
            if ( exactMatch )
            {
                DB::CategoryMatcher *exactMatcher = nullptr;
                // if andMatcher has exactMatch set, but no CategoryMatchers, then
                // matching "category / None" is what we want:
                if ( andMatcher->mp_elements.count() == 0 )
                {
                    exactMatcher = new DB::NoTagCategoryMatcher( category );
                }
                else
                {
                    ExactCategoryMatcher *noOtherMatcher = new ExactCategoryMatcher( category );
                    if ( andMatcher->mp_elements.count() == 1 )
                        noOtherMatcher->setMatcher( andMatcher->mp_elements[0] );
                    else
                        noOtherMatcher->setMatcher( andMatcher );
                    exactMatcher = noOtherMatcher;
                }
                if ( negate )
                    exactMatcher = new DB::NegationCategoryMatcher( exactMatcher );
                orMatcher->addElement( exactMatcher );
            }
            else
                if ( andMatcher->mp_elements.count() == 1 )
                    orMatcher->addElement( andMatcher->mp_elements[0] );
                else if ( andMatcher->mp_elements.count() > 1 )
                    orMatcher->addElement( andMatcher );
        }
        CategoryMatcher* matcher = nullptr;
        if ( orMatcher->mp_elements.count() == 1 )
            matcher = orMatcher->mp_elements[0];
        else if ( orMatcher->mp_elements.count() > 1 )
            matcher = orMatcher;


        if ( matcher )
        {
            m_categoryMatchers.append( matcher );
            if ( DBCategoryMatcherLog().isDebugEnabled() )
            {
                qCDebug(DBCategoryMatcherLog) << "Matching text '" << matchText << "' in category "<< category <<":";
                matcher->debug(0);
                qCDebug(DBCategoryMatcherLog) << ".";
            }
        }
    }
    m_compiled = true;
}

ImageSearchInfo::~ImageSearchInfo()
{
    deleteMatchers();
}

void ImageSearchInfo::debugMatcher() const
{
    if ( !m_compiled )
        compile();

    qCDebug(DBCategoryMatcherLog, "And:");
    for (CategoryMatcher* optionMatcher : m_categoryMatchers) {
        optionMatcher->debug(1);
    }
}

QList<QList<SimpleCategoryMatcher*> > ImageSearchInfo::query() const
{
    if ( !m_compiled )
        compile();

    // Combine _optionMachers to one list of lists in Disjunctive
    // Normal Form and return it.

    QList<CategoryMatcher*>::Iterator it  = m_categoryMatchers.begin();
    QList<QList<SimpleCategoryMatcher*> > result;
    if ( it == m_categoryMatchers.end() )
        return result;

    result = convertMatcher( *it );
    ++it;

    for( ; it != m_categoryMatchers.end(); ++it ) {
        QList<QList<SimpleCategoryMatcher*> > current = convertMatcher( *it );
        QList<QList<SimpleCategoryMatcher*> > oldResult = result;
        result.clear();

        for (QList<SimpleCategoryMatcher*> resultIt : oldResult) {
            for (QList<SimpleCategoryMatcher*> currentIt : current) {
                QList<SimpleCategoryMatcher*> tmp;
                tmp += resultIt;
                tmp += currentIt;
                result.append( tmp );
            }
        }
    }
    return result;
}

Utilities::StringSet ImageSearchInfo::findAlreadyMatched( const QString &group ) const
{
    Utilities::StringSet result;
    QString str = categoryMatchText( group );
    if ( str.contains( QString::fromLatin1( "|" ) ) ) {
        return result;
    }

    QStringList list = str.split(QString::fromLatin1( "&" ), QString::SkipEmptyParts);
    Q_FOREACH( QString part, list ) {
        QString nm = part.trimmed();
        if (! nm.contains( QString::fromLatin1( "!" ) ) )
            result.insert(nm);
    }
    return result;
}

void ImageSearchInfo::deleteMatchers() const
{
    qDeleteAll(m_categoryMatchers);
    m_categoryMatchers.clear();
}

QList<SimpleCategoryMatcher*> ImageSearchInfo::extractAndMatcher( CategoryMatcher* matcher ) const
{
    QList<SimpleCategoryMatcher*> result;

    AndCategoryMatcher* andMatcher;
    SimpleCategoryMatcher* simpleMatcher;

    if ( ( andMatcher = dynamic_cast<AndCategoryMatcher*>( matcher ) ) ) {
        for (CategoryMatcher* child : andMatcher->mp_elements) {
            SimpleCategoryMatcher* simpleMatcher = dynamic_cast<SimpleCategoryMatcher*>( child );
            Q_ASSERT( simpleMatcher );
            result.append( simpleMatcher );
        }
    }
    else if ( ( simpleMatcher = dynamic_cast<SimpleCategoryMatcher*>( matcher ) ) )
        result.append( simpleMatcher );
    else
        Q_ASSERT( false );

    return result;
}

/** Convert matcher to Disjunctive Normal Form.
 *
 * @return OR-list of AND-lists. (e.g. OR(AND(a,b),AND(c,d)))
 */
QList<QList<SimpleCategoryMatcher*> > ImageSearchInfo::convertMatcher( CategoryMatcher* item ) const
{
    QList<QList<SimpleCategoryMatcher*> > result;
    OrCategoryMatcher* orMacther;

    if ( ( orMacther = dynamic_cast<OrCategoryMatcher*>( item ) ) ) {
        for (CategoryMatcher* child : orMacther->mp_elements) {
            result.append( extractAndMatcher( child ) );
        }
    }
    else
        result.append( extractAndMatcher( item ) );
    return result;
}

ImageDate ImageSearchInfo::date() const
{
    return m_date;
}

void ImageSearchInfo::addExifSearchInfo( const Exif::SearchInfo info )
{
    m_exifSearchInfo = info;
    m_isNull = false;
}

void DB::ImageSearchInfo::renameCategory( const QString& oldName, const QString& newName )
{
    m_categoryMatchText[newName] = m_categoryMatchText[oldName];
    m_categoryMatchText.remove( oldName );
    m_compiled = false;
}

#ifdef HAVE_KGEOMAP
KGeoMap::GeoCoordinates::Pair ImageSearchInfo::regionSelection() const
{
    return m_regionSelection;
}

void ImageSearchInfo::setRegionSelection(const KGeoMap::GeoCoordinates::Pair& actRegionSelection)
{
    m_regionSelection = actRegionSelection;
    m_compiled = false;
    if (m_regionSelection.first.hasCoordinates() && m_regionSelection.second.hasCoordinates()) {
        m_isNull = false;
    }
}
#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
