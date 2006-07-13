#include "SQLImageInfo.h"
#include <qsqlquery.h>
#include "Utilities/Util.h"
#include "QueryHelper.h"
#include "QueryErrors.h"

using namespace SQLDB;

QMap<int, DB::ImageInfoPtr> SQLImageInfo::_infoPointers;
QMutex SQLImageInfo::_mutex;

DB::ImageInfoPtr SQLImageInfo::getImageInfoOf(const QString& relativeFilename)
{
    int fileId;
    try {
        fileId = QueryHelper::instance()->idForFilename(relativeFilename);
    }
    catch (Error& e) {
        // TODO: error handling
        qFatal("Error: %s", e.getMessage().local8Bit().data());
    }

    _mutex.lock();
    DB::ImageInfoPtr p = _infoPointers[fileId];
    if (p) {
        static_cast<SQLImageInfo*>(p.data())->saveChanges();
    }
    else {
        p = new SQLImageInfo(fileId);
        _infoPointers.insert(fileId, p);
    }
    _mutex.unlock();
    return p;
}

SQLDB::SQLImageInfo::~SQLImageInfo()
{
    saveChanges();
}

void SQLImageInfo::clearCache()
{
    _mutex.lock();
    for (QMap<int, DB::ImageInfoPtr>::iterator i = _infoPointers.begin();
         i != _infoPointers.end(); ++i) {
        //static_cast<SQLImageInfo*>((*i).data())->saveChanges();

        // Check if only _infoPointers has reference to the pointer.
        if ((*i).count() == 1) {
            // Then it's not needed anymore, because new one could be
            // created easily by loading from the SQL database.
            _infoPointers.remove(i);
        }
    }
    _mutex.unlock();
}

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
}

void SQLDB::SQLImageInfo::saveChanges()
{
    if (!isNull() && isDirty()) {
        QueryHelper::instance()->updateMediaItem(_fileId, *this);
        setIsDirty(false);
    }
}

DB::ImageInfo& SQLDB::SQLImageInfo::operator=( const DB::ImageInfo& other )
{
    DB::ImageInfo& tmp = DB::ImageInfo::operator=(other);
    saveChanges();
    return tmp;
}
