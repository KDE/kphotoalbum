#ifndef RESULT_H
#define RESULT_H

#include <QStringList>
#include <KSharedPtr>

namespace DB
{
class ResultId;
class ResultPtr;
class ConstResultPtr;

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

        Result();
        Result( const QList<int>& ids );

        void append( const DB::ResultId& );
        void prepend( const DB::ResultId& );
        void appendAll( const DB::Result& );
        DB::ResultId item(int index) const;
        int count() const { return size(); }  // deprecated. use size()
        int size() const;
        bool isEmpty() const;

        ConstIterator begin() const;
        ConstIterator end() const;
        void debug();

    private:
        // Noone must delete the Result directly. Only SharedPtr may.
        friend class KSharedPtr<Result>;
        friend class KSharedPtr<const Result>;
    public:
        ~Result();  // ok, seems that this doesn't work yet.
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

class ConstResultPtr :public KSharedPtr<const Result>
{
public:
    ConstResultPtr( const Result* ptr );

private:
    int count() const;
};


}


#endif /* RESULT_H */

