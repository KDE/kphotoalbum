#ifndef DB_RESULT_H
#define DB_RESULT_H

#include <QStringList>
#include <KSharedPtr>

namespace DB
{
class ResultId;
class ResultPtr;
class ConstResultPtr;

// TODO: to be renamed to ListId as per discussion in car
class Result :public KShared
    {
    public:
        class ConstIterator
        {
        public:
            ConstIterator( const Result* result, int pos );
            ConstIterator& operator++();
            DB::ResultId operator*();
            bool operator!=( const ConstIterator& other );

        private:
            const Result* _result;
            int _pos;
        };
        typedef ConstIterator const_iterator;

        Result();
        explicit Result( const QList<int>& ids );
        explicit Result( const DB::ResultId& );

        void append( const DB::ResultId& );
        void prepend( const DB::ResultId& );
        void appendAll( const DB::Result& );
        DB::ResultId at(int index) const;
        int size() const;
        bool isEmpty() const;
        int indexOf(const DB::ResultId&) const;
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
    int count() const;  // You certainly meant to call ->size() ?!
};

class ConstResultPtr :public KSharedPtr<const Result>
{
public:
    ConstResultPtr( const Result* ptr );

private:
    int count() const;  // You certainly meant to call ->size() ?!
};


}  // namespace DB


#endif /* DB_RESULT_H */

