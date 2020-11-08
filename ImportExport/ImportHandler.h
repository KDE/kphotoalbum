/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMPORTHANDLER_H
#define IMPORTHANDLER_H

#include "ImportSettings.h"

#include <DB/ImageInfoPtr.h>

#include <QEventLoop>
#include <QPointer>

namespace KIO
{
class FileCopyJob;
}
class KJob;
namespace Utilities
{
class UniqFilenameMapper;
}
class QProgressDialog;

namespace ImportExport
{
class KimFileReader;

/**
 * This class contains the business logic for the import process
 */
class ImportHandler : public QObject
{
    Q_OBJECT

public:
    ImportHandler();
    ~ImportHandler() override;
    bool exec(const ImportSettings &settings, KimFileReader *kimFileReader);

private:
    void copyFromExternal();
    void copyNextFromExternal();
    bool copyFilesFromZipFile();
    void updateDB();

private slots:
    void stopCopyingImages();
    void aCopyFailed(QStringList files);
    void aCopyJobCompleted(KJob *);

private:
    bool isImageAlreadyInDB(const DB::ImageInfoPtr &info);
    DB::ImageInfoPtr matchingInfoFromDB(const DB::ImageInfoPtr &info);
    void updateInfo(DB::ImageInfoPtr dbInfo, DB::ImageInfoPtr newInfo);
    void addNewRecord(DB::ImageInfoPtr newInfo);
    void updateCategories(DB::ImageInfoPtr XMLInfo, DB::ImageInfoPtr DBInfo, bool forceReplace);

private:
    Utilities::UniqFilenameMapper *m_fileMapper;
    bool m_finishedPressed;
    DB::ImageInfoList m_pendingCopies;
    QProgressDialog *m_progress;
    int m_totalCopied;
    KIO::FileCopyJob *m_job;
    bool m_reportUnreadableFiles;
    QPointer<QEventLoop> m_eventLoop;
    ImportSettings m_settings;
    KimFileReader *m_kimFileReader;
};

}

#endif /* IMPORTHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
