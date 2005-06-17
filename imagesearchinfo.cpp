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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "imagesearchinfo.h"
#include <qregexp.h>
#include "options.h"
#include <klocale.h>
#include "util.h"
#include <kdebug.h>
#include "optionmatcher.h"
#include "imagedb.h"
#include "imageinfo.h"
#include <kapplication.h>
#include <kconfig.h>

ImageSearchInfo::ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate,
                                  const QString& label, const QString& description )
    : _label( label ), _description( description ), _isNull( false ), _compiled( false ), _optionMatcher( 0 )
{
    if ( endDate.isNull() ) {
        _startDate = startDate;
        _endDate = startDate;
    }
    else if ( endDate <= startDate )  {
        _startDate = endDate;
        _endDate = startDate;
    }
    else {
        _startDate = startDate;
        _endDate = endDate;
    }
}

ImageDate ImageSearchInfo::startDate() const
{
    return _startDate;
}


ImageDate ImageSearchInfo::endDate() const
{
    return _endDate;
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
    : _isNull( true ), _compiled( false ), _optionMatcher( 0 )
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

    // Date
    // the search date matches the actual date if:
    // actual.start <= search.start <= actuel.end or
    // actual.start <= search.end <=actuel.end or
    // search.start <= actual.start and actual.end <= search.end

    ImageDate actualStart = info->startDate();
    ImageDate actualEnd = info->endDate();
    if ( !actualEnd.isNull() && actualEnd <= actualStart )  {
        ImageDate tmp = actualStart;
        actualStart = actualEnd;
        actualEnd = tmp;
    }
    if ( actualEnd.isNull() )
        actualEnd = actualStart;

    bool b1 =( actualStart <= _startDate && _startDate <= actualEnd );
    bool b2 =( actualStart <= _endDate && _endDate <= actualEnd );
    bool b3 = ( _startDate <= actualStart && actualEnd <= _endDate );

    ok &= ( ( b1 || b2 || b3 ) && !actualStart.isNull() ) || _startDate.isNull();


    // -------------------------------------------------- Options
    info->clearMatched();
    if ( _optionMatcher )
        ok &= _optionMatcher->eval( info );

    // -------------------------------------------------- Label
    ok &= ( _label.isEmpty() || info->label().find(_label) != -1 );

    // -------------------------------------------------- Text
    QString txt = info->description();
    QStringList list = QStringList::split( QChar(' '), _description );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        ok &= ( txt.find(*it) != -1 );
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

void ImageSearchInfo::setStartDate( const ImageDate& date )
{
    _startDate = date;
    _isNull = false;
}

void ImageSearchInfo::setEndDate( const ImageDate& date )
{
    _endDate = date;
    _isNull = false;
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
    config->setGroup( Options::instance()->groupForDatabase( QString::fromLatin1("Privacy Settings") ) );
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
    config->setGroup( Options::instance()->groupForDatabase( QString::fromLatin1("Privacy Settings") ) );
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
    _startDate = other._startDate;
    _endDate = other._endDate;
    _options = other._options;
    _label = other._label;
    _description = other._description;
    _isNull = other._isNull;
    _compiled = false;
    _optionMatcher = 0;
}

void ImageSearchInfo::compile() const
{
    delete _optionMatcher;
    OptionAndMatcher* matcher = new OptionAndMatcher;

    for( QMapConstIterator<QString,QString> it = _options.begin(); it != _options.end(); ++it ) {
        QString category = it.key();
        QString matchText = it.data();

        QStringList orParts = QStringList::split( QString::fromLatin1("|"), matchText );
        OptionOrMatcher* orMatcher = new OptionOrMatcher;

        for( QStringList::Iterator itOr = orParts.begin(); itOr != orParts.end(); ++itOr ) {
            QStringList andParts = QStringList::split( QString::fromLatin1("&"), *itOr );

            OptionAndMatcher* andMatcher = new OptionAndMatcher;
            for( QStringList::Iterator itAnd = andParts.begin(); itAnd != andParts.end(); ++itAnd ) {
                QString str = *itAnd;
                bool negate = false;
                static QRegExp regexp( QString::fromLatin1("^\\s*!\\s*(.*)$") );
                if ( regexp.exactMatch( str ) )  {
                    negate = true;
                    str = regexp.cap(1);
                }
                str = str.stripWhiteSpace();
                OptionMatcher* valueMatcher;
                if ( str == ImageDB::NONE() )
                    valueMatcher = new OptionEmptyMatcher( category, !negate );
                else
                    valueMatcher = new OptionValueMatcher( category, str, !negate );
                andMatcher->addElement( valueMatcher );
            }
            orMatcher->addElement( andMatcher );
        }
        matcher->addElement( orMatcher );
    }
    _compiled = true;
    _optionMatcher = matcher->optimize();
}

ImageSearchInfo::~ImageSearchInfo()
{
    delete _optionMatcher;
}

void ImageSearchInfo::debugMatcher() const
{
    if ( !_compiled )
        compile();
    if ( _optionMatcher ) {
        qDebug("=======================================================");
        _optionMatcher->debug(0);
        qDebug("--------------------------------------------------------");
        _optionMatcher = _optionMatcher->normalize();
        _optionMatcher = _optionMatcher->optimize();
        if ( _optionMatcher )
            _optionMatcher->debug(0);
        else
            qDebug("EMPTY MATCHER");
    }
}

OptionMatcher* ImageSearchInfo::query() const
{
    if ( !_compiled )
        compile();
    if ( isNull() )
        return 0;
    else
        return _optionMatcher;
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
