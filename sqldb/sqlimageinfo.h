#ifndef SQLIMAGEINFO_H
#define SQLIMAGEINFO_H
#include <imageinfo.h>

namespace SQLDB {
    class SQLImageInfo :public ImageInfo
    {
    public:
        SQLImageInfo( const QString& relativeFileName );
        ImageInfo& operator=( const ImageInfo& other );

    private:
        int _fileId;
    };
}


#endif /* SQLIMAGEINFO_H */

