#include "myimagecollection.h"
#include "mainview.h"
#include "imagedb.h"

MyImageCollection::MyImageCollection( Type tp )
    : _tp( tp )
{
}

QString MyImageCollection::name()
{
    qDebug("NYI: MyImageCollection::name()" );
    return QString::fromLatin1( "a name" ); // PENDING(blackie) implement
}

QString MyImageCollection::comment()
{
    return QString::null;
}

KURL::List MyImageCollection::images()
{
    switch ( _tp ) {
    case CurrentAlbum:
        return imageListToUrlList( ImageDB::instance()->currentContext( false ) );

    case CurrentView:
        return imageListToUrlList( MainView::theMainView()->currentView() );

    case CurrentSelection:
        return imageListToUrlList( MainView::theMainView()->selected() );
    }
    return KURL::List();
}

KURL::List MyImageCollection::imageListToUrlList( const ImageInfoList& imageList )
{
    KURL::List urlList;
    for( ImageInfoListIterator it( imageList ); *it; ++it ) {
        KURL url;
        url.setPath( (*it)->fileName() );
        urlList.append( url );
    }
    return urlList;
}
