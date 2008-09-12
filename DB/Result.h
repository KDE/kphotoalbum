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
class Result : public KShared {
 public:
    class ConstIterator {
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

    /** Never use Result on a stack but rather new it and assign to
     * some (Const)ResultPtr. The fact that the destructor is private will
     * remind you about this ;) */
    Result();

    /** Create a result with a list of file ids. */
    explicit Result( const QList<int>& ids );
    
    /** Convenience constructor: create a result only one ResultId. */
    explicit Result( const DB::ResultId& );
    
    void append( const DB::ResultId& );
    void prepend( const DB::ResultId& );
    
    DB::ResultId at(int index) const;
    int size() const;
    bool isEmpty() const;
    int indexOf(const DB::ResultId&) const;
    ConstIterator begin() const;
    ConstIterator end() const;
    void debug() const;
    
    /** Get the raw list for offline manipulation */
    const QList<int>& getRawFileIdList() const;
    
 private:
    // Noone must delete the Result directly. Only SharedPtr may.
    friend class KSharedPtr<Result>;
    friend class KSharedPtr<const Result>;
    ~Result();  // Don't use Q_FOREACH on DB::Result.
    
    // No implicit constructors/assignments.
    Result(const DB::Result&);
    DB::Result& operator=(const DB::Result&);
    
 private:
    QList<int> _items;
};

class ConstResultPtr;

// A reference counted pointer to a Result.
class ResultPtr : public KSharedPtr<Result> {
 public:
    ResultPtr( Result* ptr );

 private:
    ResultPtr( const ConstResultPtr& );  // Invalid to assign a ConstResultPtr.
    int count() const;  // You certainly meant to call ->size() ?!
};

// A reference counted pointer to a const Result, i.e. no non-const methods can
// be called on it. Return values of this type if you don't want the list to
// be manipulated by the caller which otherwise would be easily possible.
//
// So, the const-conversion DB::ResultPtr -> DB::ConstResultPtr is valid, but
// not the reverse.
//
// Note: it is not sufficient to just have a 'const DB::ResultPtr' (that only
// allows to call const-methods on its KShared) because it is
// possible to assign a 'const DB::ResultPtr' to a non-const DB::ResultPtr that
// would allow all write manipulations to the underlying object again.
class ConstResultPtr : public KSharedPtr<const Result> {
 public:
    ConstResultPtr( const Result* ptr );
    ConstResultPtr( const ResultPtr& nonConst );  // valid assignment.

 private:
    int count() const;  // You certainly meant to call ->size() ?!
};

}  // namespace DB


#endif /* DB_RESULT_H */

