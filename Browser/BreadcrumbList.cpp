#include "BreadcrumbList.h"
#include <QDebug>

Browser::BreadcrumbList Browser::BreadcrumbList::latest() const
{
    BreadcrumbList result;
    for ( int i = length()-1; i >=0; --i ) {
        const Breadcrumb crumb = at(i);
        const QString txt = crumb.text();
        if ( !txt.isEmpty() )
            result.prepend( crumb );

        if ( crumb.isBeginning() )
            break;
    }

    return result;
}

QString Browser::BreadcrumbList::toString() const
{
    QStringList list;
    Q_FOREACH( const Breadcrumb& item, latest() )
        list.append( item.text() );

    return list.join( QString::fromLatin1(" > ") );
}
