#include "QueryErrors.h"

using namespace SQLDB;

Error::Error(const QString& message):
    _message(message)
{
}

const QString& Error::getMessage() const
{
    return _message;
}

NotFoundError::NotFoundError(const QString& message):
    Error(message)
{
}

SQLError::SQLError():
    Error()
{
}

SQLError::SQLError(const QString& query, const QString& message):
    Error(message),
    _query(query)
{
}

const QString& SQLError::getQuery() const
{
    return _query;
}
