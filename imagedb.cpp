#include "imagedb.h"
#include <xmldb.h>
#include <klocale.h>
#include <qfileinfo.h>
#include "browser.h"

ImageDB* ImageDB::_instance = 0;


ImageDB* ImageDB::instance()
{
    if ( _instance == 0 )
        qFatal("ImageDB::instance must not be called before ImageDB::setup");
    return _instance;
}

bool ImageDB::setup( const QString& configFile )
{
    bool dirty;
    _instance = new XMLDB( configFile, &dirty );
    return dirty;
}

QString ImageDB::NONE()
{
    return i18n("**NONE**");
}

ImageInfoList ImageDB::currentScope( bool requireOnDisk ) const
{
    return search( Browser::instance()->currentContext(), requireOnDisk );
}

void ImageDB::setDateRange( const ImageDateRange& range, bool includeFuzzyCounts )
{
    _selectionRange = range;
    _includeFuzzyCounts = includeFuzzyCounts;
}

void ImageDB::clearDateRange()
{
    _selectionRange = ImageDateRange();
}

ImageInfoList ImageDB::clipboard()
{
    return _clipboard;
}

void ImageDB::setClipboard( const ImageInfoList& list )
{
    _clipboard = list;
}

bool ImageDB::isClipboardEmpty()
{
    return (_clipboard.count() == 0 );
}

