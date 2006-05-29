#ifndef SQLIMAGEINFO_H
#define SQLIMAGEINFO_H
#include "DB/ImageInfo.h"

namespace SQLDB {
    class SQLImageInfo :public DB::ImageInfo
    {
    public:
        SQLImageInfo( const QString& relativeFileName );
        DB::ImageInfo& operator=( const DB::ImageInfo& other );

    private:
        int _fileId;
    };
}


#endif /* SQLIMAGEINFO_H */

