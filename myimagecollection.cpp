#include "myimagecollection.h"
#include "mainview.h"
#include "imagedb.h"
#include "options.h"
#include <qfileinfo.h>

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

KURL MyImageCollection::path()
{
    return commonRoot();
}

KURL MyImageCollection::commonRoot()
{
    QString imgRoot = Options::instance()->imageDirectory();
    const KURL::List imgs = images();
    if ( imgs.count() == 0 )
        return imgRoot;

    QStringList res = QStringList::split( QString::fromLatin1( "/" ), QFileInfo( imgs[0].path() ).dirPath(true), true );

    for( KURL::List::ConstIterator it = imgs.begin(); it != imgs.end(); ++it ) {
        QStringList newRes;

        QStringList path = QStringList::split( QString::fromLatin1( "/" ), QFileInfo( (*it).path() ).dirPath( true ), true );
        uint i = 0;
        for ( ; i < QMIN( path.size(), res.size() ); ++i ) {
            if ( path[i] == res[i] )
                newRes.append( res[i] );
            else
                break;
        }
        res = newRes;
    }

    QString result = res.join( QString::fromLatin1( "/" ) );
    if ( result.left( imgRoot.length() ) != imgRoot ) {
        result = imgRoot;
    }

    KURL url;
    url.setPath( result );
    return url;
}

KURL MyImageCollection::uploadPath()
{
    return commonRoot();
}
