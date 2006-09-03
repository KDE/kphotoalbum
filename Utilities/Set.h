#ifndef SET_H
#define SET_H
#include <qmap.h>
#include <qvaluelist.h>

template <class TYPE>
class Set :public QMap<TYPE, TYPE>
{
public:
    Set() {}
    Set( const QValueList<TYPE>& list )
    {
        for( QValueListConstIterator<TYPE> it = list.begin(); it != list.end(); ++it ) {
            insert( *it );
        }
    }

    void insert( TYPE key )
    {
        QMap<TYPE,TYPE>::insert( key, key );
    }

    void insert( const QValueList<TYPE>& list )
    {
        for( QValueListConstIterator<TYPE> it = list.begin(); it != list.end(); ++it ) {
            insert( *it );
        }
    }

    void insert( const Set& other )
    {
        for( QValueListConstIterator<TYPE> it = other.keys().begin(); it != other.keys().end(); ++it) {
            insert( *it );
        }
    }

    QValueList<TYPE> toList() const
    {
        return QMap<TYPE,TYPE>::keys();
    }

    bool operator==( const Set<TYPE>& other )
    {
        for( typename Set<TYPE>::Iterator it = this->begin(); it != this->end(); ++it ) {
            if ( ! other.contains( *it ) )
                return false;
        }

        // The other set has the same elements as this one have, so if the number of elements are the same then
        // we can conclude the other set doesn't have any extra elements.
        return (this->count() == other.count());
    }

    const Set<TYPE> operator+=( const Set<TYPE>& other )
    {
        for( typename Set<TYPE>::ConstIterator it = other.begin(); it != other.end(); ++it )
            insert( *it );

        return *this;
    }

    const Set<TYPE> operator+( const Set<TYPE>& other ) const
    {
        Set<TYPE> res( *this );
        return res+=( other );
    }

    bool operator!=( const Set<TYPE>& other )
    {
        return !(operator==(other));
    }

    const Set<TYPE> operator-=( const Set<TYPE>& other )
    {
        for( typename Set<TYPE>::ConstIterator it = other.begin(); it != other.end(); ++it )
            this->remove( *it );
        return *this;
    }

    const Set<TYPE> operator-( const Set<TYPE>& other ) const
    {
        Set<TYPE> res(*this);
        return res.operator-=( other );
    }

};

template <class TYPE>
QDataStream& operator<<( QDataStream& stream, const Set<TYPE>& data )
{
    stream << data.count();
    for ( typename Set<TYPE>::ConstIterator itemIt = data.begin(); itemIt != data.end(); ++itemIt ) {
        stream << *itemIt;
    }
    return stream;
}

template <class TYPE>
QDataStream& operator>>( QDataStream& stream, Set<TYPE>& data )
{
    int count;
    stream >> count;
    for ( int i = 0; i < count; ++i ) {
        TYPE item;
        stream >> item;
        data.insert( item );
    }
    return stream;
}

typedef Set<QString> StringSet;


#endif /* SET_H */

