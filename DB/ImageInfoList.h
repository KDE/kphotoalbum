#ifndef IMAGEINFOLIST_H
#define IMAGEINFOLIST_H
#include <qvaluelist.h>
#include "DB/ImageInfoPtr.h"

namespace DB
{

class ImageInfoList :public QValueList<ImageInfoPtr>
{
public:
    ~ImageInfoList();
    void sortAndMergeBackIn( ImageInfoList& subListToSort );
    ImageInfoList sort() const;
    void appendList( ImageInfoList& other );
    void printItems();
    bool isSorted();
    void mergeIn( ImageInfoList list );
    void remove( ImageInfoPtr info );

private:
    bool checkIfMergeListIsContiniously( ImageInfoList& mergeList );
};

typedef QValueList<ImageInfoPtr>::Iterator ImageInfoListIterator;
typedef QValueList<ImageInfoPtr>::ConstIterator ImageInfoListConstIterator;

}

#endif /* IMAGEINFOLIST_H */

