#include "Result.h"
#include "ResultId.h"

#include <QDebug>

DB::Result::ConstIterator::ConstIterator( const Result* result, int pos )
    :_result(result), _pos(pos)
{
}

DB::Result::ConstIterator& DB::Result::ConstIterator::operator++()
{
    ++_pos;
    return *this;
}

DB::ResultId DB::Result::ConstIterator::operator*()
{
    return _result->at(_pos);
}

DB::Result::ConstIterator DB::Result::begin() const
{
    return DB::Result::ConstIterator(this, 0);
}

DB::Result::ConstIterator DB::Result::end() const
{
    return DB::Result::ConstIterator( this, size() );
}

bool DB::Result::ConstIterator::operator!=( const ConstIterator& other )
{
    return _pos != other._pos;
}

DB::ResultId DB::Result::at(int index) const
{
    return DB::ResultId(_items[index], this );
}

int DB::Result::size() const
{
    return _items.size();
}

int DB::Result::indexOf(const DB::ResultId& id) const
{
    return _items.indexOf(id.fileId());
}

DB::Result::Result( const QList<int>& ids)
    :_items(ids)
{
}

DB::Result::Result( const DB::ResultId& id) {
    _items.push_back(id.fileId());
}

DB::Result::Result()
{
}

DB::Result::~Result() 
{
}

void DB::Result::debug()
{
    qDebug() << "Size: " << size();
    qDebug() << _items;
}

void DB::Result::append( const DB::ResultId& id)
{
    _items.append(id.fileId());
}

void DB::Result::appendAll( const DB::Result& result)
{
    _items += result._items;
}


void DB::Result::prepend( const DB::ResultId& id)
{
    _items.prepend(id.fileId());
}

bool DB::Result::isEmpty() const
{
    return _items.isEmpty();
}

const QList<int>& DB::Result::getRawFileIdList() const
{
    return _items;
}

DB::ResultPtr::ResultPtr( Result* ptr )
    : KSharedPtr<Result>( ptr )
{
}

DB::ConstResultPtr::ConstResultPtr( const Result* ptr )
    : KSharedPtr<const Result>( ptr )
{
}

DB::ConstResultPtr::ConstResultPtr( const ResultPtr& nonConstPtr )
    : KSharedPtr<const Result>( nonConstPtr.data() )
{
}
