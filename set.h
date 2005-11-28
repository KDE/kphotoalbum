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
};


#endif /* SET_H */

