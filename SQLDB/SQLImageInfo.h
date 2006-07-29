#ifndef SQLIMAGEINFO_H
#define SQLIMAGEINFO_H

#include "DB/ImageInfo.h"

namespace SQLDB {
    class SQLImageInfo :public DB::ImageInfo
    {
    protected:
        friend class SQLImageInfoCollection;
        SQLImageInfo(int fileId);
        void load();
        void saveChanges();

    private:
        int _fileId;
    };
}

#endif /* SQLIMAGEINFO_H */
