#include "imagesearchinfo.h"
#include <qregexp.h>
#include "options.h"

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

    ok &= ( b1 || b2 || b3 );


    // -------------------------------------------------- Options
    QStringList grps = Options::instance()->optionGroups();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        ok &= stringMatch( *it, info );
    }

    // -------------------------------------------------- Label
    ok &= ( _label.isEmpty() || info->label().find(_label) != -1 );

    // -------------------------------------------------- Text
    QString txt = _description;
    QStringList list = QStringList::split(QRegExp(QString::fromLatin1("\\s")), txt );
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
    if ( matchText == QString::fromLatin1( "__NONE__" ) )
        return (info->optionValue( key ).count() == 0);

    QStringList orParts = QStringList::split( QString::fromLatin1("|"), matchText );
    bool orTrue = false;
    for( QStringList::Iterator itOr = orParts.begin(); itOr != orParts.end(); ++itOr ) {
        QStringList andParts = QStringList::split( QString::fromLatin1("&"), *itOr );
        bool andTrue = true;
        for( QStringList::Iterator itAnd = andParts.begin(); itAnd != andParts.end(); ++itAnd ) {
            QString str = *itAnd;
            bool negate = false;
            QRegExp regexp( QString::fromLatin1("^\\s*!\\s*(.*)$") );
            if ( regexp.exactMatch( str ) )  {
                negate = true;
                str = regexp.cap(1);
            }
            str = str.stripWhiteSpace();
            bool found = info->hasOption( key,  str );
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
}

void ImageSearchInfo::addAnd( const QString& group, const QString& value )
{
    QString val = option( group );
    if ( !val.isEmpty() )
        val += QString::fromLatin1( " & " ) + value;
    else
        val = value;

    setOption( group, val );
}

void ImageSearchInfo::setStartDate( const ImageDate& date )
{
    _startDate = date;
}

void ImageSearchInfo::setEndDate( const ImageDate& date )
{
    _endDate = date;
}

