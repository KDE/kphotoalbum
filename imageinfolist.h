#ifndef IMAGEINFOLIST_H
#define IMAGEINFOLIST_H
#include <qptrlist.h>
class ImageInfo;

class ImageInfoList :public QPtrList<ImageInfo>
{
public:
    ~ImageInfoList();
    void sortAndMergeBackIn( ImageInfoList& subListToSort );

protected:
    ImageInfoList sort() const;

private:
    bool checkIfMergeListIsContiniously( ImageInfoList& mergeList );
};

typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

#endif /* IMAGEINFOLIST_H */

