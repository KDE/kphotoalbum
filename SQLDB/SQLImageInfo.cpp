#include "SQLImageInfo.h"
#include "QueryHelper.h"

using namespace SQLDB;

SQLDB::SQLImageInfo::SQLImageInfo(QueryHelper* queryHelper, int fileId):
    DB::ImageInfo(),
    _qh(queryHelper),
    _fileId(fileId)
{
    load();
}

void SQLDB::SQLImageInfo::load()
{
    _qh->getMediaItem(_fileId, *this);
    setIsNull(false);
    setIsDirty(false);
    delaySavingChanges(false); // Doesn't save because isDirty() == false
}

void SQLDB::SQLImageInfo::saveChanges()
{
    if (!isNull() && isDirty()) {
        _qh->updateMediaItem(_fileId, *this);
        setIsDirty(false);
    }
}
