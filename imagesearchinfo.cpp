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

bool ImageSearchInfo::isNull()
{
    return _isNull;
}

bool ImageSearchInfo::match( ImageInfo* info ) const
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

void ImageSearchInfo::addAnd( const QString& group, const QString& value )
{
    QString val = option( group );
    if ( !val.isEmpty() )
        val += QString::fromLatin1( " & " ) + value;
    else
        val = value;

    setOption( group, val );
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

/**
   This method saves the current seachinfo to XML.
   This is used when saving index.xml, where the current lock is saved -- the current lock is represented by a SearchInfo.
*/
QDomElement ImageSearchInfo::toXML( QDomDocument doc )
{
    // We miss saving dates.
    QDomElement res = doc.createElement( QString::fromLatin1( "SearchInfo" ) );
    res.setAttribute( QString::fromLatin1("label"), _label );
    res.setAttribute( QString::fromLatin1("description"), _description );

    QDomElement options = doc.createElement( QString::fromLatin1( "Options" ) );
    res.appendChild( options );
    for( QMapIterator<QString,QString> it= _options.begin(); it != _options.end(); ++it ) {
        QDomElement option = doc.createElement( QString::fromLatin1("Option") );
        option.setAttribute( QString::fromLatin1("category"), it.key() );
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
            _label = elm.attribute( QString::fromLatin1( "label" ) );
            _description = elm.attribute( QString::fromLatin1( "description" ) );
            QDomNode childNode = elm.firstChild();
            if ( !childNode.isNull() && childNode.isElement() && childNode.toElement().tagName() == QString::fromLatin1( "Options" ) ) {
                QDomElement options = childNode.toElement();
                for ( QDomNode optionNode = options.firstChild(); !optionNode.isNull(); optionNode = optionNode.nextSibling() ) {
                    if ( !optionNode.isElement() )
                        continue;

                    QDomElement option = optionNode.toElement();
                    QString category = option.attribute( QString::fromLatin1( "category" ) );
                    if ( category.isNull() )
                        category = option.attribute( QString::fromLatin1( "optionGroup" ) ); // Compatible with KimDaBa 2.0
                    QString value = option.attribute( QString::fromLatin1( "value" ) );
                    if ( !category.isEmpty() )
                        _options.insert( category, value );
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
                    valueMatcher = new OptionEmptyMatcher( category );
                else
                    valueMatcher = new OptionValueMatcher( category, str );
                if ( negate )
                    valueMatcher = new OptionNotMatcher( valueMatcher );
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

