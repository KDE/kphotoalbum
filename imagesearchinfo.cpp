/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "imagesearchinfo.h"
#include <qregexp.h>
#include "options.h"
#include <klocale.h>
#include "util.h"
#include <kdebug.h>

ImageSearchInfo::ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate,
                                  const QString& label, const QString& description )
    : _label( label ), _description( description ), _isNull( false )
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
    : _isNull( true )
{
}

bool ImageSearchInfo::isNull()
{
    return _isNull;
}

bool ImageSearchInfo::match( ImageInfo* info )
{
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
    QStringList grps = Options::instance()->optionGroups();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        ok &= stringMatch( *it, info );
    }

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

bool ImageSearchInfo::stringMatch( const QString& key, ImageInfo* info )
{
    // PENDING(blackie) to simple algorithm for matching, could be improved with parentheses.
    QString matchText = _options[key];
    if ( matchText.isEmpty() )
        return true;

    // I can't make up my mind if this is too mucha a hack, so we just have
    // to see how it works.
    if ( matchText == QString::fromLatin1( "**NONE**" ) )
        return (info->optionValue( key ).count() == 0);

    QStringList orParts = QStringList::split( QString::fromLatin1("|"), matchText );
    bool orTrue = false;
    for( QStringList::Iterator itOr = orParts.begin(); itOr != orParts.end(); ++itOr ) {
        QStringList andParts = QStringList::split( QString::fromLatin1("&"), *itOr );
        bool andTrue = true;
        for( QStringList::Iterator itAnd = andParts.begin(); itAnd != andParts.end(); ++itAnd ) {
            QString str = *itAnd;
            bool negate = false;
            static QRegExp regexp( QString::fromLatin1("^\\s*!\\s*(.*)$") );
            if ( regexp.exactMatch( str ) )  {
                negate = true;
                str = regexp.cap(1);
            }
            str = str.stripWhiteSpace();
            bool found = hasOption( info, key, str );
            andTrue &= ( negate ? !found : found );
        }
        orTrue |= andTrue;
    }

    return orTrue;
}

QString ImageSearchInfo::option( const QString& name ) const
{
    return _options[name];
}

void ImageSearchInfo::setOption( const QString& name, const QString& value )
{
    _options[name] = value;
    _isNull = false;
}

void ImageSearchInfo::addAnd( const QString& group, const QString& value )
{
    QString val = option( group );
    if ( !val.isEmpty() )
        val += QString::fromLatin1( " & " ) + value;
    else
        val = value;

    setOption( group, val );
    _isNull = false;
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
            if ( txt.contains( QString::fromLatin1("|") ) )
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " %1 " ).arg( i18n("and") ) );

            else
                txt.replace( QString::fromLatin1( "&" ), QString::fromLatin1( " / " ) );

            txt.replace( QString::fromLatin1( "|" ), QString::fromLatin1( " %1 " ).arg( i18n("or") ) );
            txt.replace( QString::fromLatin1( "!" ), QString::fromLatin1( " %1 " ).arg( i18n("not") ) );
            txt.simplifyWhiteSpace();
            res += txt;
        }
    }
    return res;
}

bool ImageSearchInfo::hasOption( ImageInfo* info, const QString& key, const QString& str )
{
    QStringList list = Options::instance()->memberMap().members( key, str, true );
    bool match = info->hasOption( key,  str );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        match |= info->hasOption( key, *it );

    }
    return match;
}

void ImageSearchInfo::debug()
{
    for( QMapIterator<QString,QString> it= _options.begin(); it != _options.end(); ++it ) {
        kdDebug(50010) << it.key() << ", " << it.data() << endl;
    }
}

QDomElement ImageSearchInfo::toXML( QDomDocument doc )
{
    QDomElement res = doc.createElement( QString::fromLatin1( "SearchInfo" ) );
    // PENDING(blackie) HANDLE Dates
    res.setAttribute( QString::fromLatin1("label"), _label );
    res.setAttribute( QString::fromLatin1("description"), _description );

    QDomElement options = doc.createElement( QString::fromLatin1( "Options" ) );
    res.appendChild( options );
    for( QMapIterator<QString,QString> it= _options.begin(); it != _options.end(); ++it ) {
        QDomElement option = doc.createElement( QString::fromLatin1("Option") );
        option.setAttribute( QString::fromLatin1("optionGroup"), it.key() );
        option.setAttribute( QString::fromLatin1( "value" ), it.data() );
        options.appendChild( option );
    }
    return res;
}

void ImageSearchInfo::load( QDomElement top )
{
    _isNull = false;
    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() && node.toElement().tagName() == QString::fromLatin1( "SearchInfo" ) ) {
            QDomElement elm = node.toElement();
            // PENDING(blackie) HANDLE Dates
            _label = elm.attribute( QString::fromLatin1( "label" ) );
            _description = elm.attribute( QString::fromLatin1( "description" ) );
            QDomNode childNode = elm.firstChild();
            if ( !childNode.isNull() && childNode.isElement() && childNode.toElement().tagName() == QString::fromLatin1( "Options" ) ) {
                QDomElement options = childNode.toElement();
                for ( QDomNode optionNode = options.firstChild(); !optionNode.isNull(); optionNode = optionNode.nextSibling() ) {
                    if ( !optionNode.isElement() )
                        continue;

                    QDomElement option = optionNode.toElement();
                    QString optionGroup = option.attribute( QString::fromLatin1( "optionGroup" ) );
                    QString value = option.attribute( QString::fromLatin1( "value" ) );
                    if ( !optionGroup.isEmpty() )
                        _options.insert( optionGroup, value );
                }
                return;
            }
        }
    }
}

ImageSearchInfo::ImageSearchInfo( const ImageSearchInfo& other )
{
    _startDate = other._startDate;
    _endDate = other._endDate;
    _options = other._options;
    _label = other._label;
    _description = other._description;
    _isNull = other._isNull;
}

