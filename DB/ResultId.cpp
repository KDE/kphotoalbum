#include "ResultId.h"
#include "Result.h"

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
    Q_ASSERT(fileId != -1);
    Q_ASSERT(!_context.isNull());
}

int DB::ResultId::fileId() const
{
    return _fileId;
}

bool DB::ResultId::isNull() const {
    return _fileId == -1;
}

DB::ConstResultPtr DB::ResultId::context() const
{
    return _context;
}
