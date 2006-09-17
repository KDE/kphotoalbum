/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImageSearchInfo.h"
#include <qregexp.h>
#include "Settings/SettingsData.h"
#include <klocale.h>
#include <kdebug.h>
#include "CategoryMatcher.h"
#include "ImageDB.h"
#include "ImageInfo.h"
#include <kapplication.h>
#include <kconfig.h>
#include <config.h>

using namespace DB;

ImageSearchInfo::ImageSearchInfo( const ImageDate& date,
                                  const QString& label, const QString& description )
    : _date( date), _label( label ), _description( description ), _isNull( false ), _compiled( false )
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
    : _isNull( true ), _compiled( false )
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
#ifdef HASEXIV2
    ok = _exifSearchInfo.matches( info->fileName() );
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
    info->clearMatched();
    for( QValueList<CategoryMatcher*>::Iterator it = _optionMatchers.begin(); it != _optionMatchers.end(); ++it ) {
        ok &= (*it)->eval( info );
    }


    // -------------------------------------------------- Label
    ok &= ( _label.isEmpty() || info->label().find(_label) != -1 );

    // -------------------------------------------------- Text
    QString txt = info->description();
    if ( !txt.isEmpty() ) {
        QStringList list = QStringList::split( QChar(' '), _description );
        for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
            ok &= ( txt.find( *it, 0, false ) != -1 );
        }
    }

    return ok;
}


QString ImageSearchInfo::option( const QString& name ) const
{
    return _options[name];
}

void ImageSearchInfo::setOption( const QString& name, const QString& value )
{
    _options[name] = value;
    _isNull = false;
    _compiled = false;
}

void ImageSearchInfo::addAnd( const QString& category, const QString& value )
{
    QString val = option( category );
    if ( !val.isEmpty() )
        val += QString::fromLatin1( " & " ) + value;
    else
        val = value;

    setOption( category, val );
    _isNull = false;
    _compiled = false;
}

QString ImageSearchInfo::toString() const
{
    QString res;
    bool first = true;
    for( QMapConstIterator<QString,QString> it= _options.begin(); it != _options.end(); ++it ) {
        if ( ! it.data().isEmpty() ) {
            if ( first )
                first = false;
            else
                res += QString::fromLatin1( " / " );

            QString txt = it.data();
            if ( txt == ImageDB::NONE() )
                txt = i18n( "As in No persons, no locations etc.", "No %1" ).arg( it.key() );

            if ( txt.contains( QString::fromLatin1("|") ) )
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " %1 " ).arg( i18n("and") ) );

            else
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " / " ) );

            txt.replace( QString::fromLatin1( "|" ), QString::fromLatin1( " %1 " ).arg( i18n("or") ) );
            txt.replace( QString::fromLatin1( "!" ), QString::fromLatin1( " %1 " ).arg( i18n("not") ) );
            txt.replace( ImageDB::NONE(), i18n( "As in no other persons, or no other locations", "No other %1" ).arg( it.key() ) );
            txt.simplifyWhiteSpace();
            res += txt;
        }
    }
    return res;
}

void ImageSearchInfo::debug()
{
    for( QMapIterator<QString,QString> it= _options.begin(); it != _options.end(); ++it ) {
        kdDebug() << it.key() << ", " << it.data() << endl;
    }
}

// PENDING(blackie) move this into the Options class instead of having it here.
void ImageSearchInfo::saveLock() const
{
    KConfig* config = kapp->config();
    config->setGroup( Settings::SettingsData::instance()->groupForDatabase( QString::fromLatin1("Privacy Settings") ) );
    config->writeEntry( QString::fromLatin1("label"), _label );
    config->writeEntry( QString::fromLatin1("description"), _description );
    config->writeEntry( QString::fromLatin1("categories"), _options.keys() );
    for( QMapConstIterator<QString,QString> it= _options.begin(); it != _options.end(); ++it ) {
        config->writeEntry( it.key(), it.data() );
    }
}

ImageSearchInfo ImageSearchInfo::loadLock()
{
    KConfig* config = kapp->config();
    config->setGroup( Settings::SettingsData::instance()->groupForDatabase( QString::fromLatin1("Privacy Settings") ) );
    ImageSearchInfo info;
    info._label = config->readEntry( "label" );
    info._description = config->readEntry( "description" );
    QStringList categories = config->readListEntry( "categories" );
    for( QStringList::ConstIterator it = categories.begin(); it != categories.end(); ++it ) {
        info.setOption( *it, config->readEntry( *it ) );
    }
    return info;
}

ImageSearchInfo::ImageSearchInfo( const ImageSearchInfo& other )
{
    _date = other._date;
    _options = other._options;
    _label = other._label;
    _description = other._description;
    _isNull = other._isNull;
    _compiled = false;
#ifdef HASEXIV2
    _exifSearchInfo = other._exifSearchInfo;
#endif
}

void ImageSearchInfo::compile() const
{
#ifdef HASEXIV2
    _exifSearchInfo.search();
#endif
    deleteMatchers();

    for( QMapConstIterator<QString,QString> it = _options.begin(); it != _options.end(); ++it ) {
        QString category = it.key();
        QString matchText = it.data();

        QStringList orParts = QStringList::split( QString::fromLatin1("|"), matchText );
        OptionContainerMatcher* orMatcher = new OptionOrMatcher;

        for( QStringList::Iterator itOr = orParts.begin(); itOr != orParts.end(); ++itOr ) {
            QStringList andParts = QStringList::split( QString::fromLatin1("&"), *itOr );

            OptionContainerMatcher* andMatcher = orMatcher;
            if ( andParts.count() > 1 ) {
                andMatcher = new OptionAndMatcher;
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
                str = str.stripWhiteSpace();
                CategoryMatcher* valueMatcher;
                if ( str == ImageDB::NONE() )
                    valueMatcher = new OptionEmptyMatcher( category, !negate );
                else
                    valueMatcher = new OptionValueMatcher( category, str, !negate );
                andMatcher->addElement( valueMatcher );
            }
        }
        if ( orMatcher->_elements.count() == 1 )
            _optionMatchers.append( orMatcher->_elements[0] );
        else if ( orMatcher->_elements.count() > 1 )
            _optionMatchers.append( orMatcher );
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
    for( QValueList<CategoryMatcher*>::Iterator it = _optionMatchers.begin(); it != _optionMatchers.end(); ++it ) {
        (*it)->debug(1);
    }
}

QValueList< QValueList<OptionSimpleMatcher*> > ImageSearchInfo::query() const
{
    if ( !_compiled )
        compile();

    // Combine _optionMachers to one list of lists in Disjunctive
    // Normal Form and return it.

    QValueList<CategoryMatcher*>::Iterator it  = _optionMatchers.begin();
    QValueList< QValueList<OptionSimpleMatcher*> > result;
    if ( it == _optionMatchers.end() )
        return result;

    result = convertMatcher( *it );
    ++it;

    for( ; it != _optionMatchers.end(); ++it ) {
        QValueList< QValueList<OptionSimpleMatcher*> > current = convertMatcher( *it );
        QValueList< QValueList<OptionSimpleMatcher*> > oldResult = result;
        result.clear();

        for( QValueList< QValueList<OptionSimpleMatcher*> >::Iterator resultIt = oldResult.begin();
             resultIt != oldResult.end(); ++resultIt ) {

            for( QValueList< QValueList<OptionSimpleMatcher*> >::Iterator currentIt = current.begin();
                 currentIt != current.end(); ++currentIt ) {

                QValueList<OptionSimpleMatcher*> tmp;
                tmp += (*resultIt);
                tmp += (*currentIt);
                result.append( tmp );
            }
        }
    }
    return result;
}

QDict<void> ImageSearchInfo::findAlreadyMatched( const QString &group ) const
{
    QDict<void> map;
    QString str = option( group );
    if ( str.contains( QString::fromLatin1( "|" ) ) ) {
        return map;
    }

    QStringList list = QStringList::split( QString::fromLatin1( "&" ), str );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString nm = (*it).stripWhiteSpace();
        if (! nm.contains( QString::fromLatin1( "!" ) ) )
            map.insert( nm, (void*) 0x1 /* something different from 0x0 */ );
    }
    return map;
}

void ImageSearchInfo::deleteMatchers() const
{
    for( QValueList<CategoryMatcher*>::Iterator it = _optionMatchers.begin(); it != _optionMatchers.end();  ) {
        CategoryMatcher* matcher = *it;
        ++it;
        delete matcher;
    }
    _optionMatchers.clear();
}

QValueList<OptionSimpleMatcher*> ImageSearchInfo::extractAndMatcher( CategoryMatcher* matcher ) const
{
    QValueList< OptionSimpleMatcher*> result;

    OptionAndMatcher* andMatcher;
    OptionSimpleMatcher* simpleMatcher;

    if ( ( andMatcher = dynamic_cast<OptionAndMatcher*>( matcher ) ) ) {
        for( QValueList<CategoryMatcher*>::Iterator childIt = andMatcher->_elements.begin();
             childIt != andMatcher->_elements.end(); ++childIt ) {
            OptionSimpleMatcher* simpleMatcher = dynamic_cast<OptionSimpleMatcher*>( *childIt );
            Q_ASSERT( simpleMatcher );
            result.append( simpleMatcher );
        }
    }
    else if ( ( simpleMatcher = dynamic_cast<OptionSimpleMatcher*>( matcher ) ) )
        result.append( simpleMatcher );
    else
        Q_ASSERT( false );

    return result;
}

/** Convert matcher to Disjunctive Normal Form.
 *
 * @return OR-list of AND-lists. (e.g. OR(AND(a,b),AND(c,d)))
 */
QValueList< QValueList<OptionSimpleMatcher*> > ImageSearchInfo::convertMatcher( CategoryMatcher* item ) const
{
    QValueList< QValueList<OptionSimpleMatcher*> > result;
    OptionOrMatcher* orMacther;

    if ( ( orMacther = dynamic_cast<OptionOrMatcher*>( item ) ) ) {
        for( QValueList<CategoryMatcher*>::Iterator childIt = orMacther->_elements.begin();
             childIt != orMacther->_elements.end(); ++childIt ) {
            result.append( extractAndMatcher( *childIt ) );
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

#ifdef HASEXIV2
void ImageSearchInfo::addExifSearchInfo( const Exif::SearchInfo info )
{
    _exifSearchInfo = info;
    _isNull = false;
}
#endif

void DB::ImageSearchInfo::renameCategory( const QString& oldName, const QString& newName )
{
    _options[newName] = _options[oldName];
    _options.remove( oldName );
    _compiled = false;
}
