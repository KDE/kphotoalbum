#ifndef SQLDB_QUERYERRORS_H
#define SQLDB_QUERYERRORS_H

#include <qstring.h>

namespace SQLDB
{

class Error
{
public:
    Error(const QString& message=QString::null);
    const QString& getMessage() const;

private:
    QString _message;
};

class NotFoundError : public Error
{
public:
    NotFoundError(const QString& message=QString::null);
};

class SQLError : public Error
{
public:
    SQLError();
    SQLError(const QString& query, const QString& message=QString::null);
    const QString& getQuery() const;

private:
    QString _query;
};

}

#endif /* SQLDB_QUERYERRORS_H */
