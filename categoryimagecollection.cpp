#include "categoryimagecollection.h"
#include <klocale.h>
#include "imagedb.h"
CategoryImageCollection::CategoryImageCollection( const ImageSearchInfo& context, const QString& optionGroup,
                                                  const QString& value )
    : MyImageCollection( CategoryImageCollection::SubClass ), _context( context ), _optionGroup( optionGroup ),
      _value( value )
{
}

QString CategoryImageCollection::name()
{
    if ( _value == QString::fromLatin1( "**NONE**" ) )
        return i18n( "Example: No Persons", "No %1" ).arg( _optionGroup );
    else
        return _value;
}

KURL::List CategoryImageCollection::images()
{
    ImageSearchInfo context( _context );
    context.addAnd( _optionGroup, _value );
    ImageInfoList list = ImageDB::instance()->images( context, true );
    return imageListToUrlList( list );
}
