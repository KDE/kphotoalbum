#include "imagedb.h"
#include <xmldb.h>
#include <klocale.h>

ImageDB* ImageDB::_instance = 0;


ImageDB* ImageDB::instance()
{
    if ( _instance == 0 )
        qFatal("ImageDB::instance must not be called before ImageDB::setup");
    return _instance;
}

bool ImageDB::setup( const QDomElement& options, const QDomElement& top, const QDomElement& blockList, const QDomElement& memberGroups )
{
    bool dirty;
    _instance = new XMLDB( options, top, blockList, memberGroups, &dirty );
    return dirty;
}

QString ImageDB::NONE()
{
    return i18n("**NONE**");
}



