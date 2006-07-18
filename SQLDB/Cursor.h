#include "QueryErrors.h"

namespace SQLDB
{
    using KexiDB::RowData;

    class Cursor
    {
    public:
        Cursor(KexiDB::Cursor* cursor): _cursor(cursor)
        {
            if (!_cursor)
                throw Error(/* TODO: type and message */);
        }

        ~Cursor()
        {
            _cursor->connection()->deleteCursor(_cursor);
        }

        inline
        void selectFirstRow() { _cursor->moveFirst(); }

        inline
        void selectNextRow() { _cursor->moveNext(); }

        inline
        bool rowExists() const { return !_cursor->eof(); }

        inline
        void getCurrentRow(RowData& data) { _cursor->storeCurrentRow(data); }

        inline
        RowData getCurrentRow()
        {
            RowData data;
            _cursor->storeCurrentRow(data);
            return data;
        }

    private:
        KexiDB::Cursor* _cursor;
    };
}
