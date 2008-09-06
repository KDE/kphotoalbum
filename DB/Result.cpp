#include "Result.h"
#include <QDebug>

DB::Result::ConstIterator::ConstIterator( const Result* result, int id )
    :_result(result), _id(id)
{
}

DB::Result::ConstIterator& DB::Result::ConstIterator::operator++()
{
    ++_id;
    return *this;
}

DB::ResultId DB::Result::ConstIterator::operator*()
{
    return _result->item(_id);
}

DB::Result::ConstIterator DB::Result::begin() const
{
    return DB::Result::ConstIterator(this,0);
}

DB::Result::ConstIterator DB::Result::end() const
{
    return DB::Result::ConstIterator( this, count() );
}

bool DB::Result::ConstIterator::operator!=( const ConstIterator& other )
{
    return _id != other._id;
}

DB::ResultId DB::Result::item(int index) const
{
    return DB::ResultId(_items[index], *this );
}

int DB::Result::count() const
{
    return _items.count();
}

DB::Result::Result( const QList<int>& ids)
    :_items(ids)
{
}

DB::Result::Result()
{
}

void DB::Result::debug()
{
    qDebug() << "Count: " << count();
    qDebug() << _items;
}

void DB::Result::append( DB::ResultId id)
{
    _items.append(id.fileId());
}

bool DB::Result::isEmpty() const
{
    return _items.isEmpty();
}

