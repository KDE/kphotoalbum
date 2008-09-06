#ifndef RESULT_H
#define RESULT_H

#include <QStringList>
#include <KSharedPtr>
#include "DB/ResultId.h"

namespace DB
{
class Result :public KShared
    {
    public:
        class ConstIterator
        {
        public:
            ConstIterator( const Result* result, int id );
            ConstIterator& operator++();
            DB::ResultId operator*();
            bool operator!=( const ConstIterator& other );

        private:
            const Result* _result;
            int _id;
        };


        Result( const QList<int>& ids );
        Result();
        void append( int id );

        DB::ResultId item(int index) const;
        int count() const;

        ConstIterator begin() const;
        ConstIterator end() const;
        void debug();

    private:
        QList<int> _items;
    };

    typedef KSharedPtr<Result> ResultPtr;
}


#endif /* RESULT_H */

