#ifndef IMAGEINFOPTR_H
#define IMAGEINFOPTR_H
#include <ksharedptr.h>

namespace DB
{
    class ImageInfo;
    typedef KSharedPtr<ImageInfo> ImageInfoPtr;
}

#endif /* IMAGEINFOPTR_H */

