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

    bool operator!=( const Set<TYPE>& other )
    {
        return !(operator==(other));
    }
};


#endif /* SET_H */

