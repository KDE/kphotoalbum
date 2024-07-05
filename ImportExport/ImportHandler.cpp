// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImportHandler.h"

#include "ImportSettings.h"
#include "KimFileReader.h"
#include "Logging.h"

#include <Browser/BrowserWidget.h>
#include <DB/Category.h>
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/MD5.h>
#include <DB/MD5Map.h>
#include <MainWindow/Window.h>
#include <Utilities/UniqFilenameMapper.h>

#include <KConfigGroup>
#include <KIO/StatJob>
#include <KJobUiDelegate>
#include <KJobWidgets>
#include <KLocalizedString>
#include <QApplication>
#include <QFile>
#include <QProgressDialog>
#include <kio/job.h>
#include <kio_version.h>
#include <KIO/FileCopyJob>
#include <kmessagebox.h>
#include <kwidgetsaddons_version.h>
#include <memory>

using namespace ImportExport;

ImportExport::ImportHandler::ImportHandler()
    : m_fileMapper(nullptr)
    , m_finishedPressed(false)
    , m_progress(nullptr)
    , m_totalCopied(0)
    , m_job(nullptr)
    , m_reportUnreadableFiles(true)
    , m_eventLoop(new QEventLoop)
    , m_settings()
    , m_kimFileReader(nullptr)

{
}

ImportHandler::~ImportHandler()
{
    delete m_fileMapper;
    delete m_eventLoop;
}

bool ImportExport::ImportHandler::exec(const ImportSettings &settings, KimFileReader *kimFileReader)
{
    m_settings = settings;
    m_kimFileReader = kimFileReader;
    m_finishedPressed = true;
    delete m_fileMapper;
    m_fileMapper = new Utilities::UniqFilenameMapper(m_settings.destination());
    bool ok;
    // copy images
    if (m_settings.externalSource()) {
        copyFromExternal();

        // If none of the images were to be copied, then we flushed the loop before we got started, in that case, don't start the loop.
        qCDebug(ImportExportLog) << "Copying" << m_pendingCopies.count() << "files from external source...";
        if (m_pendingCopies.count() > 0)
            ok = m_eventLoop->exec();
        else
            ok = false;
    } else {
        ok = copyFilesFromZipFile();
        if (ok)
            updateDB();
    }
    if (m_progress)
        delete m_progress;

    return ok;
}

void ImportExport::ImportHandler::copyFromExternal()
{
    m_pendingCopies = m_settings.selectedImages();
    m_totalCopied = 0;
    m_progress = new QProgressDialog(MainWindow::Window::theMainWindow());
    m_progress->setWindowTitle(i18nc("@title:window", "Copying Images"));
    m_progress->setMinimum(0);
    m_progress->setMaximum(2 * m_pendingCopies.count());
    m_progress->show();
    connect(m_progress, &QProgressDialog::canceled, this, &ImportHandler::stopCopyingImages);
    if (m_pendingCopies.isEmpty()) {
        qCDebug(ImportExportLog) << "No images selected for import!";
        m_eventLoop->exit(false);
        return;
    }
    copyNextFromExternal();
}

void ImportExport::ImportHandler::copyNextFromExternal()
{
    // this function must not be called without a next image
    Q_ASSERT(!m_pendingCopies.isEmpty());
    DB::ImageInfoPtr info = m_pendingCopies[0];

    if (isImageAlreadyInDB(info)) {
        qCDebug(ImportExportLog) << info->fileName().relative() << "is already in database.";
        aCopyJobCompleted(nullptr);
        return;
    }

    const DB::FileName fileName = info->fileName();

    bool succeeded = false;
    QStringList tried;

    // First search for images next to the .kim file
    // Second search for images base on the image root as specified in the .kim file
    const QList<QUrl> searchUrls {
        m_settings.kimFile().adjusted(QUrl::RemoveFilename), m_settings.baseURL().adjusted(QUrl::RemoveFilename)
    };
    for (const QUrl &url : searchUrls) {
        QUrl src(url);
        src.setPath(src.path() + fileName.relative());

#if KIO_VERSION < QT_VERSION_CHECK(5, 69, 0)
        std::unique_ptr<KIO::StatJob> statJob { KIO::stat(src, KIO::StatJob::SourceSide, 0 /* just query for existence */) };
#else
        std::unique_ptr<KIO::StatJob> statJob { KIO::statDetails(src, KIO::StatJob::SourceSide, KIO::StatDetail::StatNoDetails) };
#endif
        KJobWidgets::setWindow(statJob.get(), MainWindow::Window::theMainWindow());
        if (statJob->exec()) {
            QUrl dest = QUrl::fromLocalFile(m_fileMapper->uniqNameFor(fileName));
            m_job = KIO::file_copy(src, dest, -1, KIO::HideProgressInfo);
            connect(m_job, &KIO::FileCopyJob::result, this, &ImportHandler::aCopyJobCompleted);
            succeeded = true;
            qCDebug(ImportExportLog) << "Copying" << src << "to" << dest;
            break;
        } else
            tried << src.toDisplayString();
    }

    if (!succeeded)
        aCopyFailed(tried);
}

bool ImportExport::ImportHandler::copyFilesFromZipFile()
{
    DB::ImageInfoList images = m_settings.selectedImages();

    m_totalCopied = 0;
    m_progress = new QProgressDialog(MainWindow::Window::theMainWindow());
    m_progress->setWindowTitle(i18nc("@title:window", "Copying Images"));
    m_progress->setMinimum(0);
    m_progress->setMaximum(2 * m_pendingCopies.count());
    m_progress->show();

    for (DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it) {
        if (!isImageAlreadyInDB(*it)) {
            const DB::FileName fileName = (*it)->fileName();
            QByteArray data = m_kimFileReader->loadImage(fileName.relative());
            if (data.isNull())
                return false;
            QString newName = m_fileMapper->uniqNameFor(fileName);

            QFile out(newName);
            if (!out.open(QIODevice::WriteOnly)) {
                KMessageBox::error(MainWindow::Window::theMainWindow(), i18n("Error when writing image %1", newName));
                return false;
            }
            out.write(data.constData(), data.size());
            out.close();
        }

        qApp->processEvents();
        m_progress->setValue(++m_totalCopied);
        if (m_progress->wasCanceled()) {
            return false;
        }
    }
    return true;
}

void ImportExport::ImportHandler::updateDB()
{
    disconnect(m_progress, &QProgressDialog::canceled, this, &ImportHandler::stopCopyingImages);
    m_progress->setLabelText(i18n("Updating Database"));
    int len = Settings::SettingsData::instance()->imageDirectory().length();
    // image directory is always a prefix of destination
    if (len == m_settings.destination().length())
        len = 0;
    else
        qCDebug(ImportExportLog)
            << "Re-rooting of ImageInfos from " << Settings::SettingsData::instance()->imageDirectory()
            << " to " << m_settings.destination();

    // Run though all images
    DB::ImageInfoList images = m_settings.selectedImages();
    for (DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it) {
        DB::ImageInfoPtr info = *it;
        if (len != 0) {
            // exchange prefix:
            QString name = m_settings.destination() + info->fileName().absolute().mid(len);
            qCDebug(ImportExportLog) << info->fileName().absolute() << " -> " << name;
            info->setFileName(DB::FileName::fromAbsolutePath(name));
        }

        if (isImageAlreadyInDB(info)) {
            qCDebug(ImportExportLog) << "Updating ImageInfo for " << info->fileName().absolute();
            updateInfo(matchingInfoFromDB(info), info);
        } else {
            qCDebug(ImportExportLog) << "Adding ImageInfo for " << info->fileName().absolute();
            addNewRecord(info);
        }

        m_progress->setValue(++m_totalCopied);
        if (m_progress->wasCanceled())
            break;
    }

    Browser::BrowserWidget::instance()->home();
}

void ImportExport::ImportHandler::stopCopyingImages()
{
    m_job->kill();
}

void ImportExport::ImportHandler::aCopyFailed(QStringList files)
{
    if (m_reportUnreadableFiles) {
        const QString warnMessage = i18n("Cannot copy from any of the following locations:");
        const QString title = i18nc("@title", "Copy failed");
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        const auto answer = KMessageBox::warningTwoActionsCancelList(m_progress,
                                                                     warnMessage,
                                                                     files,
                                                                     title,
                                                                     KStandardGuiItem::cont(),
                                                                     KGuiItem(i18nc("@action:button", "Continue without Asking")));
        if (answer == KMessageBox::ButtonCode::SecondaryAction) {
#else
        const auto answer = KMessageBox::warningYesNoCancelList(m_progress, warnMessage,
                                                                files, title, KStandardGuiItem::cont(), KGuiItem(i18nc("@action:button", "Continue without Asking")));
        if (answer == KMessageBox::No) {
#endif
            m_reportUnreadableFiles = false;
        } else if (answer == KMessageBox::Cancel) {
            // This might be late -- if we managed to copy some files, we will
            // just throw away any changes to the DB, but some new image files
            // might be in the image directory...
            m_eventLoop->exit(false);
            m_pendingCopies.pop_front();
            return;
        }
    }
    aCopyJobCompleted(nullptr);
}

void ImportExport::ImportHandler::aCopyJobCompleted(KJob *job)
{
    qCDebug(ImportExportLog) << "CopyJob" << job << "completed.";
    m_pendingCopies.pop_front();
    if (job && job->error()) {
        job->uiDelegate()->showErrorMessage();
        m_eventLoop->exit(false);
    } else if (m_pendingCopies.count() == 0) {
        updateDB();
        m_eventLoop->exit(true);
    } else if (m_progress->wasCanceled()) {
        m_eventLoop->exit(false);
    } else {
        m_progress->setValue(++m_totalCopied);
        copyNextFromExternal();
    }
}

bool ImportExport::ImportHandler::isImageAlreadyInDB(const DB::ImageInfoPtr &info)
{
    return DB::ImageDB::instance()->md5Map()->contains(info->MD5Sum());
}

DB::ImageInfoPtr ImportExport::ImportHandler::matchingInfoFromDB(const DB::ImageInfoPtr &info)
{
    const DB::FileName name = DB::ImageDB::instance()->md5Map()->lookup(info->MD5Sum());
    return DB::ImageDB::instance()->info(name);
}

/**
 * Merge the ImageInfo data from the kim file into the existing ImageInfo.
 */
void ImportExport::ImportHandler::updateInfo(DB::ImageInfoPtr dbInfo, DB::ImageInfoPtr newInfo)
{
    if (dbInfo->label() != newInfo->label() && m_settings.importAction(QString::fromLatin1("*Label*")) == ImportSettings::Replace)
        dbInfo->setLabel(newInfo->label());

    if (dbInfo->description().simplified() != newInfo->description().simplified()) {
        if (m_settings.importAction(QString::fromLatin1("*Description*")) == ImportSettings::Replace)
            dbInfo->setDescription(newInfo->description());
        else if (m_settings.importAction(QString::fromLatin1("*Description*")) == ImportSettings::Merge)
            dbInfo->setDescription(dbInfo->description() + QString::fromLatin1("<br/><br/>") + newInfo->description());
    }

    if (dbInfo->angle() != newInfo->angle() && m_settings.importAction(QString::fromLatin1("*Orientation*")) == ImportSettings::Replace)
        dbInfo->setAngle(newInfo->angle());

    if (dbInfo->date() != newInfo->date() && m_settings.importAction(QString::fromLatin1("*Date*")) == ImportSettings::Replace)
        dbInfo->setDate(newInfo->date());

    updateCategories(newInfo, dbInfo, false);
}

void ImportExport::ImportHandler::addNewRecord(DB::ImageInfoPtr info)
{
    const DB::FileName importName = info->fileName();

    DB::ImageInfoPtr updateInfo(new DB::ImageInfo(importName, info->mediaType(), DB::FileInformation::Ignore));
    updateInfo->setLabel(info->label());
    updateInfo->setDescription(info->description());
    updateInfo->setDate(info->date());
    updateInfo->setAngle(info->angle());
    updateInfo->setMD5Sum(DB::MD5Sum(updateInfo->fileName()));

    DB::ImageInfoList list;
    list.append(updateInfo);
    DB::ImageDB::instance()->addImages(list);

    updateCategories(info, updateInfo, true);
}

void ImportExport::ImportHandler::updateCategories(DB::ImageInfoPtr XMLInfo, DB::ImageInfoPtr DBInfo, bool forceReplace)
{
    // Run though the categories
    const QList<CategoryMatchSetting> matches = m_settings.categoryMatchSetting();

    for (const CategoryMatchSetting &match : matches) {
        QString XMLCategoryName = match.XMLCategoryName();
        QString DBCategoryName = match.DBCategoryName();
        ImportSettings::ImportAction action = m_settings.importAction(DBCategoryName);

        const Utilities::StringSet items = XMLInfo->itemsOfCategory(XMLCategoryName);
        DB::CategoryPtr DBCategoryPtr = DB::ImageDB::instance()->categoryCollection()->categoryForName(DBCategoryName);

        if (!forceReplace && action == ImportSettings::Replace)
            DBInfo->setCategoryInfo(DBCategoryName, Utilities::StringSet());

        if (action == ImportSettings::Merge || action == ImportSettings::Replace || forceReplace) {
            for (const QString &item : items) {
                if (match.XMLtoDB().contains(item)) {
                    DBInfo->addCategoryInfo(DBCategoryName, match.XMLtoDB()[item]);
                    DBCategoryPtr->addItem(match.XMLtoDB()[item]);
                }
            }
        }
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ImportHandler.cpp"
