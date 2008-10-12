#include "ResultId.h"
#include "Result.h"
#include "ImageDB.h"

DB::ResultId const DB::ResultId::null;

DB::ResultId::ResultId()
    : _rawId()
    , _context()
{
    Q_ASSERT(isNull());
}

DB::ResultId::ResultId(DB::RawId rawId, const Result& context)
    : _rawId(rawId)
    , _context(context)
{
    Q_ASSERT(!isNull());
    Q_ASSERT(!_context.isEmpty());
}

DB::RawId DB::ResultId::rawId() const
{
    return _rawId;
}

bool DB::ResultId::isNull() const {
    Q_ASSERT(_rawId == DB::RawId() || toInt(_rawId) >= 1);
    return _rawId == DB::RawId();
}

DB::ImageInfoPtr DB::ResultId::fetchInfo() const {
    if (isNull()) return ImageInfoPtr(NULL);
    return DB::ImageDB::instance()->info(*this);
}

const DB::Result& DB::ResultId::context() const
{
    return _context;
}

