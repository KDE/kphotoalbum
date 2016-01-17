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
#include "ValueCategoryMatcher.h"
#include "ExactCategoryMatcher.h"
#include "NegationCategoryMatcher.h"
#include "NoTagCategoryMatcher.h"
#include "AndCategoryMatcher.h"
#include "ContainerCategoryMatcher.h"
#include "OrCategoryMatcher.h"
#include <qregexp.h>
#include "Settings/SettingsData.h"
#include <klocale.h>
#include <kdebug.h>
#include "CategoryMatcher.h"
#include "ImageDB.h"
#include <kapplication.h>
#include <config-kpa-exiv2.h>
#include <kconfiggroup.h>
#include "ImageManager/RawImageDecoder.h"
using namespace DB;

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description )
    : m_date( date), m_label( label ), m_description( description ), m_rating( -1 ), m_megapixel( 0 ), m_ratingSearchMode( 0 ), m_searchRAW( false ), m_isNull( false ), m_compiled( false )
{
}

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description,
                  const QString& fnPattern )
    : m_date( date), m_label( label ), m_description( description ), m_fnPattern( fnPattern ), m_rating( -1 ), m_megapixel( 0 ), m_ratingSearchMode( 0 ), m_searchRAW( false ), m_isNull( false ), m_compiled( false )
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
    : m_rating( -1 ), m_megapixel( 0 ), m_ratingSearchMode( 0 ), m_searchRAW( false ), m_isNull( true ), m_compiled( false )
{
}

bool ImageSearchInfo::isNull() const
{
    return m_isNull;
}

bool ImageSearchInfo::match( ImageInfoPtr info ) const
{
    if ( m_isNull )
        return true;

    if ( !m_compiled )
        compile();

    bool ok = true;
#ifdef HAVE_EXIV2
    ok = m_exifSearchInfo.matches( info->fileName() );
#endif

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
        // actual.start <= search.start <= actuel.end or
        // actual.start <= search.end <=actuel.end or
        // search.start <= actual.start and actual.end <= search.end

        bool b1 =( actualStart <= m_date.start() && m_date.start() <= actualEnd );
        bool b2 =( actualStart <= m_date.end() && m_date.end() <= actualEnd );
        bool b3 = ( m_date.start() <= actualStart && ( actualEnd <= m_date.end() || m_date.end().isNull() ) );

        ok = ok && ( ( b1 || b2 || b3 ) );
    } else if ( !m_date.end().isNull() ) {
        bool b1 = ( actualStart <= m_date.end() && m_date.end() <= actualEnd );
        bool b2 = ( actualEnd <= m_date.end() );
        ok = ok && ( ( b1 || b2 ) );
    }

    // -------------------------------------------------- Options
    // alreadyMatched map is used to make it possible to search for
    // Jesper & None
    QMap<QString, StringSet> alreadyMatched;
    for (CategoryMatcher* optionMatcher : m_categoryMatchers) {
        ok = ok && optionMatcher->eval(info, alreadyMatched);
    }


    // -------------------------------------------------- Label
    ok = ok && ( m_label.isEmpty() || info->label().indexOf(m_label) != -1 );

    // -------------------------------------------------- RAW
    ok = ok && ( m_searchRAW == false || ImageManager::RAWImageDecoder::isRAW( info->fileName()) );

    // -------------------------------------------------- Rating

    //ok = ok && (_rating == -1 ) || ( _rating == info->rating() );
    if (m_rating != -1) {
    switch( m_ratingSearchMode ) {
        case 1:
        // Image rating at least selected
        ok = ok && ( m_rating <= info->rating() );
        break;
        case 2:
        // Image rating less than selected
        ok = ok && ( m_rating >= info->rating() );
        break;
        case 3:
        // Image rating not equal
        ok = ok && ( m_rating != info->rating() );
        break;
        default:
            ok = ok && ((m_rating == -1 ) || ( m_rating == info->rating() ));
        break;
    }
    }


    // -------------------------------------------------- Resolution
    if ( m_megapixel )
        ok = ok && ( m_megapixel * 1000000 <= info->size().width() * info->size().height() );

    // -------------------------------------------------- Text
    QString txt = info->description();
    if ( !m_description.isEmpty() ) {
        QStringList list = m_description.split(QChar::fromLatin1(' '), QString::SkipEmptyParts);
        Q_FOREACH( const QString &word, list ) {
            ok = ok && ( txt.indexOf( word, 0, Qt::CaseInsensitive ) != -1 );
        }
    }

    // -------------------------------------------------- File name pattern
    ok = ok && ( m_fnPattern.isEmpty() ||
        m_fnPattern.indexIn( info->fileName().relative() ) != -1 );


#ifdef HAVE_KGEOMAP
    // Search for GPS Position
    if (ok && m_usingRegionSelection) {
        ok = ok && info->coordinates().hasCoordinates();
        if (ok) {
            float infoLat = info->coordinates().lat();
            float infoLon = info->coordinates().lon();
            ok = ok
                 && m_regionSelectionMinLat <= infoLat
                 && infoLat                 <= m_regionSelectionMaxLat
                 && m_regionSelectionMinLon <= infoLon
                 && infoLon                 <= m_regionSelectionMaxLon;
        }
    }
#endif

    return ok;
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
}

void ImageSearchInfo::setRating( short rating )
{
  m_rating = rating;
  m_isNull = false;
  m_compiled = false;
}

void ImageSearchInfo::setMegaPixel( short megapixel )
{
  m_megapixel = megapixel;
}

void ImageSearchInfo::setSearchMode(int index)
{
  m_ratingSearchMode = index;
}

void ImageSearchInfo::setSearchRAW( bool searchRAW )
{
  m_searchRAW = searchRAW;
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
        kDebug() << it.key() << ", " << it.value();
    }
}

// PENDING(blackie) move this into the Options class instead of having it here.
void ImageSearchInfo::saveLock() const
{
    KConfigGroup config = KGlobal::config()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings"));
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
    KConfigGroup config = KGlobal::config()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings" ));
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
    m_searchRAW = other.m_searchRAW;
#ifdef HAVE_EXIV2
    m_exifSearchInfo = other.m_exifSearchInfo;
#endif
#ifdef HAVE_KGEOMAP
    m_regionSelection = other.m_regionSelection;
#endif

}

void ImageSearchInfo::compile() const
{
#ifdef HAVE_EXIV2
    m_exifSearchInfo.search();
#endif
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
            m_categoryMatchers.append( matcher );
#ifdef DEBUG_CATEGORYMATCHERS
        if ( matcher )
        {
            qDebug() << "Matching text '" << matchText << "' in category "<< category <<":";
            matcher->debug(0);
            qDebug() << ".";
        }
#endif
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

    qDebug("And:");
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

#ifdef HAVE_EXIV2
void ImageSearchInfo::addExifSearchInfo( const Exif::SearchInfo info )
{
    m_exifSearchInfo = info;
    m_isNull = false;
}
#endif

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
}
#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
