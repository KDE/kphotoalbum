#ifndef IMAGEINFOLIST_H
#define IMAGEINFOLIST_H
#include <qptrlist.h>
class ImageInfo;

class ImageInfoList :public QPtrList<ImageInfo>
{
public:
    ~ImageInfoList();
    void sortAndMergeBackIn( ImageInfoList& subListToSort );
    ImageInfoList sort() const;
    void appendList( ImageInfoList& other );
    void printItems();
    bool isSorted();
    void mergeIn( ImageInfoList list );

private:
    bool checkIfMergeListIsContiniously( ImageInfoList& mergeList );
};

typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

#endif /* IMAGEINFOLIST_H */

