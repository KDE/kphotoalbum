#include "Result.h"
#include "ResultId.h"
#include "ImageInfo.h"

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

bool DB::Result::ConstIterator::operator==( const ConstIterator& other )
{
    return _pos == other._pos;
}

bool DB::Result::ConstIterator::operator!=( const ConstIterator& other )
{
    return _pos != other._pos;
}

DB::ResultId DB::Result::at(int index) const
{
    return DB::ResultId(_items[index], *this);
}

int DB::Result::size() const
{
    return _items.size();
}

int DB::Result::indexOf(const DB::ResultId& id) const
{
    return _items.indexOf(id.rawId());
}

DB::Result::Result(const QList<DB::RawId>& ids)
    :_items(ids)
{
}

DB::Result::Result( const DB::ResultId& id) {
    _items.push_back(id.rawId());
}

DB::Result::Result()
{
}

void DB::Result::debug() const
{
    qDebug() << "Size: " << size();
    qDebug() << _items;
}

void DB::Result::append( const DB::ResultId& id)
{
    _items.append(id.rawId());
}

void DB::Result::prepend( const DB::ResultId& id)
{
    _items.prepend(id.rawId());
}

void DB::Result::removeAll(const DB::ResultId& id)
{
    _items.removeAll(id.rawId());
}

bool DB::Result::isEmpty() const
{
    return _items.isEmpty();
}

QList<DB::ImageInfoPtr> DB::Result::fetchInfos() const
{
    QList<DB::ImageInfoPtr> infos;
    for (const_iterator i = begin(); i != end(); ++i)
        infos.push_back((*i).fetchInfo());
    return infos;
}

const QList<DB::RawId>& DB::Result::rawIdList() const
{
    return _items;
}

DB::Result DB::Result::reversed() const
{
    Result res;
    Q_FOREACH(ResultId id, *this) {
        res.prepend(id);
    }
    return res;
}
