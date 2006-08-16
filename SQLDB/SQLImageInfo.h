#ifndef SQLIMAGEINFO_H
#define SQLIMAGEINFO_H

#include "DB/ImageInfo.h"
#include "QueryHelper.h"

namespace SQLDB {
    class SQLImageInfo :public DB::ImageInfo
    {
    protected:
        friend class SQLImageInfoCollection;
        SQLImageInfo(QueryHelper* queryHelper, int fileId);
        void load();
        void saveChanges();

        QueryHelper* _qh;

    private:
        int _fileId;
    };
}

#endif /* SQLIMAGEINFO_H */
