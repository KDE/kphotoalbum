#include "ResultId.h"
#include "Result.h"
#include "ImageDB.h"

DB::ResultId const DB::ResultId::null;

DB::ResultId::ResultId()
    : _rawId(-1)
    , _context(0)
{
}

DB::ResultId::ResultId(int rawId, const ConstResultPtr& context)
    : _rawId(rawId)
    , _context(context)
{
    Q_ASSERT(rawId >= 0);
    Q_ASSERT(!_context.isNull());
}

int DB::ResultId::rawId() const
{
    return _rawId;
}

bool DB::ResultId::isNull() const {
    return _rawId == -1;
}

DB::ImageInfoPtr DB::ResultId::fetchInfo() const {
    if (isNull()) return ImageInfoPtr(NULL);
    return DB::ImageDB::instance()->info(*this);
}

DB::ConstResultPtr DB::ResultId::context() const
{
    return _context;
}

