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
        typedef ConstIterator const_iterator;

        Result( const QList<int>& ids );
        Result();
        void append( DB::ResultId );

        DB::ResultId item(int index) const;
        int count() const;
        bool isEmpty() const;

        ConstIterator begin() const;
        ConstIterator end() const;
        void debug();

    private:
        QList<int> _items;
    };

class ResultPtr :public KSharedPtr<Result>
{
public:
    ResultPtr( Result* ptr );

private:
    int count() const;
};


}


#endif /* RESULT_H */

