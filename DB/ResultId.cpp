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
    , _context(!context.isNull() ? context : new Result(QList<int>() << fileId))
{
    Q_ASSERT(!_context.isNull());
}

int DB::ResultId::fileId() const
{
    return _fileId;
}

bool DB::ResultId::isNull() const {
    return _fileId == -1;
}
