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
