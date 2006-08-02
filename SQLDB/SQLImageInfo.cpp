#include "SQLImageInfo.h"
#include "QueryHelper.h"

using namespace SQLDB;

SQLDB::SQLImageInfo::SQLImageInfo(int fileId):
    DB::ImageInfo(),
    _fileId(fileId)
{
    load();
}

void SQLDB::SQLImageInfo::load()
{
    QueryHelper::instance()->getMediaItem(_fileId, *this);
    setIsNull(false);
    setIsDirty(false);
    delaySavingChanges(false); // Doesn't save because isDirty() == false
}

void SQLDB::SQLImageInfo::saveChanges()
{
    if (!isNull() && isDirty()) {
        QueryHelper::instance()->updateMediaItem(_fileId, *this);
        setIsDirty(false);
    }
}
