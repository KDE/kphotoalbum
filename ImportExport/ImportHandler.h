#ifndef IMPORTHANDLER_H
#define IMPORTHANDLER_H

#include "ImportSettings.h"
#include <QEventLoop>
#include "DB/ImageInfoPtr.h"

namespace KIO { class FileCopyJob; }
class KJob;
namespace Utilities { class UniqFilenameMapper; }
class QProgressDialog;

namespace ImportExport {
class KimFileReader;

/**
 * This class contains the business logic for the import process
 */
class ImportHandler :public QObject
{
    Q_OBJECT

public:
    ImportHandler();
    ~ImportHandler();
    bool exec( const ImportSettings& settings, KimFileReader* kimFileReader );

private:
    void copyFromExternal();
    void copyNextFromExternal();
    bool copyFilesFromZipFile();
    void updateDB();

private slots:
    void stopCopyingImages();
    void aCopyFailed( QStringList files );
    void aCopyJobCompleted( KJob* );

private:
    bool isImageAlreadyInDB( const DB::ImageInfoPtr& info );
    DB::ImageInfoPtr matchingInfoFromDB( const DB::ImageInfoPtr& info );
    void updateInfo( DB::ImageInfoPtr dbInfo, DB::ImageInfoPtr newInfo );
    void addNewRecord( DB::ImageInfoPtr newInfo );
    void updateCategories( DB::ImageInfoPtr XMLInfo, DB::ImageInfoPtr DBInfo, bool forceReplace );

private:
    Utilities::UniqFilenameMapper* m_fileMapper;
    bool m_finishedPressed;
    DB::ImageInfoList _pendingCopies;
    QProgressDialog* _progress;
    int _totalCopied;
    KIO::FileCopyJob* _job;
    bool _reportUnreadableFiles;
    QEventLoop m_eventLoop;
    ImportSettings m_settings;
    KimFileReader* m_kimFileReader;
};

}

#endif /* IMPORTHANDLER_H */

