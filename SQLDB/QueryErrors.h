#ifndef SQLDB_QUERYERRORS_H
#define SQLDB_QUERYERRORS_H

#include <qstring.h>

#define ERROR_CLASS(X,Y) \
struct X: public Y\
{\
    X(const QString& message=QString::null): Y(message) {}\
    QString name() const { return QString::fromLatin1(#X); }\
}

#define ERROR_CLASS2(X,Y) \
struct X: public Y\
{\
    X(const QString& queryLine=QString::null,\
      const QString& message=QString::null): Y(queryLine, message) {}\
    QString name() const { return QString::fromLatin1(#X); }\
}

namespace SQLDB
{
    class Error
    {
    public:
        Error(const QString& message=QString::null): _message(message) {}
        const QString& message() const { return _message; }
        virtual QString name() const { return QString::fromLatin1("Error"); }

    private:
        QString _message;
    };

    ERROR_CLASS(NotFoundError, Error);
    ERROR_CLASS(DriverNotFoundError, NotFoundError);
    ERROR_CLASS(DataNotFoundError, NotFoundError);
    ERROR_CLASS(RowNotFoundError, DataNotFoundError);
    ERROR_CLASS(EntryNotFoundError, DataNotFoundError);

    ERROR_CLASS(InitializationError, Error);
    ERROR_CLASS(DriverLoadError, InitializationError);

    ERROR_CLASS(ConnectionError, Error);
    ERROR_CLASS(ConnectionCreateError, ConnectionError);
    ERROR_CLASS(ConnectionOpenError, ConnectionError);
    ERROR_CLASS(ConnectionCloseError, ConnectionError);

    ERROR_CLASS(DatabaseError, Error);
    ERROR_CLASS(DatabaseCreateError, DatabaseError);
    ERROR_CLASS(DatabaseOpenError, DatabaseError);
    ERROR_CLASS(DatabaseSchemaError, DatabaseOpenError);

    ERROR_CLASS(TableCreateError, Error);

    ERROR_CLASS(TransactionError, Error);
    ERROR_CLASS(TransactionCreateError, TransactionError);
    ERROR_CLASS(TransactionCommitError, TransactionError);

    class SQLError: public Error
    {
    public:
        SQLError(const QString& queryLine=QString::null,
                 const QString& message=QString::null):
            Error(message), _queryLine(queryLine) {}
        const QString& queryLine() const { return _queryLine; }
        QString name() const { return QString::fromLatin1("SQLError"); }

    private:
        QString _queryLine;
    };

    ERROR_CLASS2(QueryError, SQLError);
    ERROR_CLASS2(StatementError, SQLError);
}

#undef ERROR_CLASS
#undef ERROR_CLASS2

#endif /* SQLDB_QUERYERRORS_H */
