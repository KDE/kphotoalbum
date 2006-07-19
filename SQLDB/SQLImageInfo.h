#ifndef SQLIMAGEINFO_H
#define SQLIMAGEINFO_H

#include "DB/ImageInfo.h"

namespace SQLDB {
    class SQLImageInfoCollection;

    class SQLImageInfo :public DB::ImageInfo
    {
    public:
        virtual ~SQLImageInfo();
        DB::ImageInfo& operator=( const DB::ImageInfo& other );
        void saveChanges();

    protected:
        friend class SQLImageInfoCollection;
        SQLImageInfo(int fileId);
        void load();

    private:
        int _fileId;
    };
}

#endif /* SQLIMAGEINFO_H */
