/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include "NoOtherItemsCategoryMatcher.h"
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
using namespace DB;

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description )
    : _date( date), _label( label ), _description( description ), _rating( -1 ), _isNull( false ), _compiled( false )
{
}

QString ImageSearchInfo::label() const
{
    return _label;
}

QString ImageSearchInfo::description() const
{
    return _description;
}

ImageSearchInfo::ImageSearchInfo()
    : _rating( -1 ), _isNull( true ), _compiled( false )
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
    ok = _exifSearchInfo.matches( info->fileName(DB::AbsolutePath) );
#endif

    if ( !_date.start().isNull() ) {
        // Date
        // the search date matches the actual date if:
        // actual.start <= search.start <= actuel.end or
        // actual.start <= search.end <=actuel.end or
        // search.start <= actual.start and actual.end <= search.end

        QDateTime actualStart = info->date().start();
        QDateTime actualEnd = info->date().end();
        if ( actualEnd <= actualStart )  {
            QDateTime tmp = actualStart;
            actualStart = actualEnd;
        actualEnd = tmp;
        }

        bool b1 =( actualStart <= _date.start() && _date.start() <= actualEnd );
        bool b2 =( actualStart <= _date.end() && _date.end() <= actualEnd );
        bool b3 = ( _date.start() <= actualStart && actualEnd <= _date.end() );

        ok &= ( ( b1 || b2 || b3 ) );
    }

    // -------------------------------------------------- Options
    // alreadyMatched map is used to make it possible to search for
    // Jesper & None
    QMap<QString, StringSet> alreadyMatched;
    Q_FOREACH(CategoryMatcher* optionMatcher, _categoryMatchers) {
        ok &= optionMatcher->eval(info, alreadyMatched);
    }


    // -------------------------------------------------- Label
    ok &= ( _label.isEmpty() || info->label().indexOf(_label) != -1 );

    // -------------------------------------------------- Rating

    ok &= (_rating == -1 ) || ( _rating == info->rating() );

    // -------------------------------------------------- Text
    QString txt = info->description();
    if ( !_description.isEmpty() ) {
        QStringList list = _description.split(QChar::fromLatin1(' '), QString::SkipEmptyParts);
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            ok &= ( txt.indexOf( *it, 0, Qt::CaseInsensitive ) != -1 );
        }
    }

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
                            "the best I can do - Jesper.", "No %1" ).arg( it.key() );

            if ( txt.contains( QString::fromLatin1("|") ) )
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " %1 " ).arg( i18n("and") ) );

            else
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " / " ) );

            txt.replace( QString::fromLatin1( "|" ), QString::fromLatin1( " %1 " ).arg( i18n("or") ) );
            txt.replace( QString::fromLatin1( "!" ), QString::fromLatin1( " %1 " ).arg( i18n("not") ) );
            txt.replace( ImageDB::NONE(), i18nc( "As in no other persons, or no other locations. "
                                                "I do realize that translators may have problem with this, "
                                                "but I need some how to indicate the category, and users may create their own categories, so this is "
                                                "the best I can do - Jesper.", "No other %1" ).arg( it.key() ) );
            txt.simplified();
            res += txt;
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
    _isNull = other._isNull;
    _compiled = false;
    _rating = other._rating;
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

            DB::ContainerCategoryMatcher* andMatcher = orMatcher;
            if ( andParts.count() > 1 ) {
                andMatcher = new DB::AndCategoryMatcher;
                orMatcher->addElement( andMatcher );
            }


            for( QStringList::Iterator itAnd = andParts.begin(); itAnd != andParts.end(); ++itAnd ) {
                QString str = *itAnd;
                bool negate = false;
                static QRegExp regexp( QString::fromLatin1("^\\s*!\\s*(.*)$") );
                if ( regexp.exactMatch( str ) )  {
                    negate = true;
                    str = regexp.cap(1);
                }
                str = str.trimmed();
                CategoryMatcher* valueMatcher;
                if ( str == ImageDB::NONE() )
                    valueMatcher = new DB::NoOtherItemsCategoryMatcher( category, !negate );
                else
                    valueMatcher = new DB::ValueCategoryMatcher( category, str, !negate );
                andMatcher->addElement( valueMatcher );
            }
        }
        CategoryMatcher* matcher = 0;
        if ( orMatcher->_elements.count() == 1 )
            matcher = orMatcher->_elements[0];
        else if ( orMatcher->_elements.count() > 1 )
            matcher = orMatcher;


        if ( matcher ) {
            matcher->finalize();
            _categoryMatchers.append( matcher );
        }
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

Q3Dict<void> ImageSearchInfo::findAlreadyMatched( const QString &group ) const
{
    Q3Dict<void> map;
    QString str = categoryMatchText( group );
    if ( str.contains( QString::fromLatin1( "|" ) ) ) {
        return map;
    }

    QStringList list = str.split(QString::fromLatin1( "&" ), QString::SkipEmptyParts);
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString nm = (*it).trimmed();
        if (! nm.contains( QString::fromLatin1( "!" ) ) )
            map.insert( nm, (void*) 0x1 /* something different from 0x0 */ );
    }
    return map;
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
