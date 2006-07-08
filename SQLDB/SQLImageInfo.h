#ifndef SQLIMAGEINFO_H
#define SQLIMAGEINFO_H
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include <qmap.h>
#include <qmutex.h>

namespace SQLDB {
    class SQLImageInfo :public DB::ImageInfo
    {
    public:
        static DB::ImageInfoPtr getImageInfoOf(const QString& relativeFilename);
        virtual ~SQLImageInfo();
        DB::ImageInfo& operator=( const DB::ImageInfo& other );
        void saveChanges();
        static void clearCache();

    protected:
        SQLImageInfo(int fileId);
        void load();

    private:
        int _fileId;
        static QMap<int, DB::ImageInfoPtr> _infoPointers;
        static QMutex _mutex;
    };
}


#endif /* SQLIMAGEINFO_H */

