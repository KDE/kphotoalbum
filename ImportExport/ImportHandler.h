#ifndef IMPORTHANDLER_H
#define IMPORTHANDLER_H

#include "ImportSettings.h"
#include <QEventLoop>
#include "Utilities/Util.h"
#include <kio/job.h>

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
    Utilities::UniqNameMap m_nameMap;
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

