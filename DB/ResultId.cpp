#include "ResultId.h"
#include "Result.h"
#include "ImageDB.h"

DB::ResultId const DB::ResultId::null;

DB::ResultId::ResultId()
    : _fileId(-1)
    , _context(0)
{
}

DB::ResultId::ResultId(int fileId, const ConstResultPtr& context)
    : _fileId(fileId)
    , _context(context)
{
    Q_ASSERT(fileId >= 0);
    Q_ASSERT(!_context.isNull());
}

int DB::ResultId::fileId() const
{
    return _fileId;
}

bool DB::ResultId::isNull() const {
    return _fileId == -1;
}

DB::ImageInfoPtr DB::ResultId::fetchInfo() const {
    if (isNull()) return ImageInfoPtr(NULL);
    return DB::ImageDB::instance()->info(*this);
}

DB::ConstResultPtr DB::ResultId::context() const
{
    return _context;
}

