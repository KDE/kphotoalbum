#ifndef IMPORTHANDLER_H
#define IMPORTHANDLER_H

#include "ImportSettings.h"
#include <QEventLoop>
#include "Utilities/Util.h"
#include <kio/job.h>

class QProgressDialog;

namespace ImportExport {

class ImportDialog;

class ImportHandler :public QObject
{
    Q_OBJECT

public:
    ImportHandler( ImportDialog* import );
    bool exec( const ImportSettings& settings );

public: // JKP
    void copyFromExternal();
    void copyNextFromExternal();
    bool copyFilesFromZipFile();
    void updateDB();

private slots:
    void stopCopyingImages();
    void aCopyFailed( QStringList files );
    void aCopyJobCompleted( KJob* );

public: // JKP
    ImportDialog* m_import;
    Utilities::UniqNameMap m_nameMap;
    bool m_finishedPressed;
    DB::ImageInfoList _pendingCopies;
    QProgressDialog* _progress;
    int _totalCopied;
    KIO::FileCopyJob* _job;
    bool _reportUnreadableFiles;
    QEventLoop m_eventLoop;
    ImportSettings m_settings;
};

}

#endif /* IMPORTHANDLER_H */

