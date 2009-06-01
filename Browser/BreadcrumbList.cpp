#include "BreadcrumbList.h"

QStringList Browser::BreadcrumbList::latest() const
{
    QStringList result;
    for ( int i = length()-1; i >=0; --i ) {
        const Breadcrumb crumb = at(i);
        const QString txt = crumb.text();
        if ( !txt.isEmpty() )
            result.prepend( txt );

        if ( crumb.isBeginning() )
            break;
    }
    return result;
}

QString Browser::BreadcrumbList::toString() const
{
    return latest().join( QString::fromLatin1(" / ") );
}
