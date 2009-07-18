#ifndef DB_RESULT_H
#define DB_RESULT_H

#include "RawId.h"
#include "ImageInfoPtr.h"

namespace DB
{
class ResultId;

/** List of media item ids.
 *
 * Instances are implicitly shared, so they can be passed by value in
 * constant time.
 *
 * This class implements forward iterator API and it is possible to
 * use Q_FOREACH to iterate over the ResultId objects in the list.
 *
 * Iterating example:
 * \code
 * DB::Result list = getSomeIdList();
 * Q_FOREACH(DB::ResultId id, list) {
 *     doSomethingWithMediaId(id);
 * }
 * \endcode
 *
 * \todo Rename to IdList as per discussion in car
 */
class Result
{
 public:
    class ConstIterator {
    public:
        ConstIterator& operator++();
        DB::ResultId operator*();
        bool operator==( const ConstIterator& other );
        bool operator!=( const ConstIterator& other );

    private:
        friend class Result;
        ConstIterator( const Result* result, int pos );

        const Result* _result;
        int _pos;
    };
    typedef ConstIterator const_iterator;

    Result();

    /** Create a result with a list of raw ids. */
    explicit Result( const QList<DB::RawId>& ids );

    /** Convenience constructor: create a result only one ResultId. */
    explicit Result( const DB::ResultId& );

    void append( const DB::ResultId& );
    void prepend( const DB::ResultId& );
    void removeAll(const DB::ResultId&);
    Result reversed() const;

    DB::ResultId at(int index) const;
    int size() const;
    bool isEmpty() const;
    int indexOf(const DB::ResultId&) const;
    ConstIterator begin() const;
    ConstIterator end() const;
    void debug() const;

    QList<DB::ImageInfoPtr> fetchInfos() const;

    /** Get the raw list for offline manipulation */
    const QList<DB::RawId>& rawIdList() const;

 private:
    QList<DB::RawId> _items;
};

}  // namespace DB


#endif /* DB_RESULT_H */

