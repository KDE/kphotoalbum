/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
    : _date( date), _label( label ), _description( description ), _rating( -1 ), _megapixel( 0 ), ratingSearchMode( 0 ), _searchRAW( false ), _isNull( false ), _compiled( false )
{
}

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description,
                  const QString& fnPattern )
    : _date( date), _label( label ), _description( description ), _fnPattern( fnPattern ), _rating( -1 ), _megapixel( 0 ), ratingSearchMode( 0 ), _searchRAW( false ), _isNull( false ), _compiled( false )
{
}

QString ImageSearchInfo::label() const
{
    return _label;
}

QRegExp ImageSearchInfo::fnPattern() const
{
    return _fnPattern;
}

QString ImageSearchInfo::description() const
{
    return _description;
}

ImageSearchInfo::ImageSearchInfo()
    : _rating( -1 ), _megapixel( 0 ), ratingSearchMode( 0 ), _searchRAW( false ), _isNull( true ), _compiled( false )
{
}

bool ImageSearchInfo::isNull() const
{
    return _isNull;
}

bool ImageSearchInfo::match( ImageInfoPtr info ) const
{
    if ( _isNull )
        return true;

    if ( !_compiled )
        compile();

    bool ok = true;
#ifdef HAVE_EXIV2
    ok = _exifSearchInfo.matches( info->fileName() );
#endif

    QDateTime actualStart = info->date().start();
    QDateTime actualEnd = info->date().end();
    if ( actualEnd <= actualStart )  {
        QDateTime tmp = actualStart;
        actualStart = actualEnd;
        actualEnd = tmp;
    }

    if ( !_date.start().isNull() ) {
        // Date
        // the search date matches the actual date if:
        // actual.start <= search.start <= actuel.end or
        // actual.start <= search.end <=actuel.end or
        // search.start <= actual.start and actual.end <= search.end

        bool b1 =( actualStart <= _date.start() && _date.start() <= actualEnd );
        bool b2 =( actualStart <= _date.end() && _date.end() <= actualEnd );
        bool b3 = ( _date.start() <= actualStart && ( actualEnd <= _date.end() || _date.end().isNull() ) );

        ok = ok && ( ( b1 || b2 || b3 ) );
    } else if ( !_date.end().isNull() ) {
        bool b1 = ( actualStart <= _date.end() && _date.end() <= actualEnd );
        bool b2 = ( actualEnd <= _date.end() );
        ok = ok && ( ( b1 || b2 ) );
    }

    // -------------------------------------------------- Options
    // alreadyMatched map is used to make it possible to search for
    // Jesper & None
    QMap<QString, StringSet> alreadyMatched;
    Q_FOREACH(CategoryMatcher* optionMatcher, _categoryMatchers) {
        ok = ok && optionMatcher->eval(info, alreadyMatched);
    }


    // -------------------------------------------------- Label
    ok = ok && ( _label.isEmpty() || info->label().indexOf(_label) != -1 );

    // -------------------------------------------------- RAW
    ok = ok && ( _searchRAW == false || ImageManager::RAWImageDecoder::isRAW( info->fileName()) );

    // -------------------------------------------------- Rating

    //ok = ok && (_rating == -1 ) || ( _rating == info->rating() );
    if (_rating != -1) {
    switch( ratingSearchMode ) {
        case 1:
        // Image rating at least selected
        ok = ok && ( _rating <= info->rating() );
        break;
        case 2:
        // Image rating less than selected
        ok = ok && ( _rating >= info->rating() );
        break;
        case 3:
        // Image rating not equal
        ok = ok && ( _rating != info->rating() );
        break;
        default:
            ok = ok && ((_rating == -1 ) || ( _rating == info->rating() ));
        break;
    }
    }


    // -------------------------------------------------- Resolution
    if ( _megapixel )
        ok = ok && ( _megapixel * 1000000 <= info->size().width() * info->size().height() );

    // -------------------------------------------------- Text
    QString txt = info->description();
    if ( !_description.isEmpty() ) {
        QStringList list = _description.split(QChar::fromLatin1(' '), QString::SkipEmptyParts);
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            ok = ok && ( txt.indexOf( *it, 0, Qt::CaseInsensitive ) != -1 );
        }
    }

    // -------------------------------------------------- File name pattern
    ok = ok && ( _fnPattern.isEmpty() ||
        _fnPattern.indexIn( info->fileName().relative() ) != -1 );

    return ok;
}


QString ImageSearchInfo::categoryMatchText( const QString& name ) const
{
    return _categoryMatchText[name];
}

void ImageSearchInfo::setCategoryMatchText( const QString& name, const QString& value )
{
    _categoryMatchText[name] = value;
    _isNull = false;
    _compiled = false;
}

void ImageSearchInfo::addAnd( const QString& category, const QString& value )
{
    QString val = categoryMatchText( category );
    if ( !val.isEmpty() )
        val += QString::fromLatin1( " & " ) + value;
    else
        val = value;

    setCategoryMatchText( category, val );
    _isNull = false;
    _compiled = false;
}

void ImageSearchInfo::setRating( short rating )
{
  _rating = rating;
  _isNull = false;
  _compiled = false;
}

void ImageSearchInfo::setMegaPixel( short megapixel )
{
  _megapixel = megapixel;
}

void ImageSearchInfo::setSearchMode(int index)
{
  ratingSearchMode = index;
}

void ImageSearchInfo::setSearchRAW( bool searchRAW )
{
  _searchRAW = searchRAW;
}


QString ImageSearchInfo::toString() const
{
    QString res;
    bool first = true;
    for( QMap<QString,QString>::ConstIterator it= _categoryMatchText.begin(); it != _categoryMatchText.end(); ++it ) {
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
    for( QMap<QString,QString>::Iterator it= _categoryMatchText.begin(); it != _categoryMatchText.end(); ++it ) {
        kDebug() << it.key() << ", " << it.value();
    }
}

// PENDING(blackie) move this into the Options class instead of having it here.
void ImageSearchInfo::saveLock() const
{
    KConfigGroup config = KGlobal::config()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings"));
    config.writeEntry( QString::fromLatin1("label"), _label );
    config.writeEntry( QString::fromLatin1("description"), _description );
    config.writeEntry( QString::fromLatin1("categories"), _categoryMatchText.keys() );
    for( QMap<QString,QString>::ConstIterator it= _categoryMatchText.begin(); it != _categoryMatchText.end(); ++it ) {
        config.writeEntry( it.key(), it.value() );
    }
    config.sync();
}

ImageSearchInfo ImageSearchInfo::loadLock()
{
    KConfigGroup config = KGlobal::config()->group( Settings::SettingsData::instance()->groupForDatabase( "Privacy Settings" ));
    ImageSearchInfo info;
    info._label = config.readEntry( "label" );
    info._description = config.readEntry( "description" );
    QStringList categories = config.readEntry<QStringList>( QString::fromLatin1("categories"), QStringList() );
    for( QStringList::ConstIterator it = categories.constBegin(); it != categories.constEnd(); ++it ) {
        info.setCategoryMatchText( *it, config.readEntry<QString>( *it, QString() ) );
    }
    return info;
}

ImageSearchInfo::ImageSearchInfo( const ImageSearchInfo& other )
{
    _date = other._date;
    _categoryMatchText = other._categoryMatchText;
    _label = other._label;
    _description = other._description;
    _fnPattern = other._fnPattern;
    _isNull = other._isNull;
    _compiled = false;
    _rating = other._rating;
    ratingSearchMode = other.ratingSearchMode;
    _megapixel = other._megapixel;
    _searchRAW = other._searchRAW;
#ifdef HAVE_EXIV2
    _exifSearchInfo = other._exifSearchInfo;
#endif
}

void ImageSearchInfo::compile() const
{
#ifdef HAVE_EXIV2
    _exifSearchInfo.search();
#endif
    deleteMatchers();

    for( QMap<QString,QString>::ConstIterator it = _categoryMatchText.begin(); it != _categoryMatchText.end(); ++it ) {
        QString category = it.key();
        QString matchText = it.value();

        QStringList orParts = matchText.split(QString::fromLatin1("|"), QString::SkipEmptyParts);
        DB::ContainerCategoryMatcher* orMatcher = new DB::OrCategoryMatcher;

        for( QStringList::Iterator itOr = orParts.begin(); itOr != orParts.end(); ++itOr ) {
            QStringList andParts = (*itOr).split(QString::fromLatin1("&"), QString::SkipEmptyParts);

            DB::ContainerCategoryMatcher* andMatcher;
            bool exactMatch=false;
            bool negate = false;
            andMatcher = new DB::AndCategoryMatcher;

            for( QStringList::Iterator itAnd = andParts.begin(); itAnd != andParts.end(); ++itAnd ) {
                QString str = *itAnd;
                static QRegExp regexp( QString::fromLatin1("^\\s*!\\s*(.*)$") );
                if ( regexp.exactMatch( str ) )
                { // str is preceeded with NOT
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
                DB::CategoryMatcher *exactMatcher = 0;
                // if andMatcher has exactMatch set, but no CategoryMatchers, then
                // matching "category / None" is what we want:
                if ( andMatcher->_elements.count() == 0 )
                {
                    exactMatcher = new DB::NoTagCategoryMatcher( category );
                }
                else
                {
                    ExactCategoryMatcher *noOtherMatcher = new ExactCategoryMatcher( category );
                    if ( andMatcher->_elements.count() == 1 )
                        noOtherMatcher->setMatcher( andMatcher->_elements[0] );
                    else
                        noOtherMatcher->setMatcher( andMatcher );
                    exactMatcher = noOtherMatcher;
                }
                if ( negate )
                    exactMatcher = new DB::NegationCategoryMatcher( exactMatcher );
                orMatcher->addElement( exactMatcher );
            }
            else
                if ( andMatcher->_elements.count() == 1 )
                    orMatcher->addElement( andMatcher->_elements[0] );
                else if ( andMatcher->_elements.count() > 1 )
                    orMatcher->addElement( andMatcher );
        }
        CategoryMatcher* matcher = 0;
        if ( orMatcher->_elements.count() == 1 )
            matcher = orMatcher->_elements[0];
        else if ( orMatcher->_elements.count() > 1 )
            matcher = orMatcher;


        if ( matcher )
            _categoryMatchers.append( matcher );
#ifdef DEBUG_CATEGORYMATCHERS
        if ( matcher )
        {
            qDebug() << "Matching text '" << matchText << "' in category "<< category <<":";
            matcher->debug(0);
            qDebug() << ".";
        }
#endif
    }
    _compiled = true;
}

ImageSearchInfo::~ImageSearchInfo()
{
    deleteMatchers();
}

void ImageSearchInfo::debugMatcher() const
{
    if ( !_compiled )
        compile();

    qDebug("And:");
    Q_FOREACH(CategoryMatcher* optionMatcher, _categoryMatchers) {
        optionMatcher->debug(1);
    }
}

QList<QList<SimpleCategoryMatcher*> > ImageSearchInfo::query() const
{
    if ( !_compiled )
        compile();

    // Combine _optionMachers to one list of lists in Disjunctive
    // Normal Form and return it.

    QList<CategoryMatcher*>::Iterator it  = _categoryMatchers.begin();
    QList<QList<SimpleCategoryMatcher*> > result;
    if ( it == _categoryMatchers.end() )
        return result;

    result = convertMatcher( *it );
    ++it;

    for( ; it != _categoryMatchers.end(); ++it ) {
        QList<QList<SimpleCategoryMatcher*> > current = convertMatcher( *it );
        QList<QList<SimpleCategoryMatcher*> > oldResult = result;
        result.clear();

        Q_FOREACH(QList<SimpleCategoryMatcher*> resultIt, oldResult) {
            Q_FOREACH(QList<SimpleCategoryMatcher*> currentIt, current) {
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
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString nm = (*it).trimmed();
        if (! nm.contains( QString::fromLatin1( "!" ) ) )
            result.insert(nm);
    }
    return result;
}

void ImageSearchInfo::deleteMatchers() const
{
    Q_FOREACH(CategoryMatcher* matcher, _categoryMatchers) {
        delete matcher;
    }
    _categoryMatchers.clear();
}

QList<SimpleCategoryMatcher*> ImageSearchInfo::extractAndMatcher( CategoryMatcher* matcher ) const
{
    QList<SimpleCategoryMatcher*> result;

    AndCategoryMatcher* andMatcher;
    SimpleCategoryMatcher* simpleMatcher;

    if ( ( andMatcher = dynamic_cast<AndCategoryMatcher*>( matcher ) ) ) {
        Q_FOREACH(CategoryMatcher* child, andMatcher->_elements) {
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
        Q_FOREACH(CategoryMatcher* child, orMacther->_elements) {
            result.append( extractAndMatcher( child ) );
        }
    }
    else
        result.append( extractAndMatcher( item ) );
    return result;
}

ImageDate ImageSearchInfo::date() const
{
    return _date;
}

#ifdef HAVE_EXIV2
void ImageSearchInfo::addExifSearchInfo( const Exif::SearchInfo info )
{
    _exifSearchInfo = info;
    _isNull = false;
}
#endif

void DB::ImageSearchInfo::renameCategory( const QString& oldName, const QString& newName )
{
    _categoryMatchText[newName] = _categoryMatchText[oldName];
    _categoryMatchText.remove( oldName );
    _compiled = false;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
