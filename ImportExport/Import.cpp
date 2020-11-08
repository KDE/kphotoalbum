/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "Import.h"

#include "ImportDialog.h"
#include "ImportHandler.h"
#include "KimFileReader.h"

#include <MainWindow/Window.h>

#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <QFileDialog>
#include <QTemporaryFile>

using namespace ImportExport;

void Import::imageImport()
{
    QUrl url = QFileDialog::getOpenFileUrl(
        nullptr, /*parent*/
        i18n("KPhotoAlbum Export Files"), /*caption*/
        QUrl(), /* directory */
        i18n("KPhotoAlbum import files") + QString::fromLatin1("(*.kim)") /*filter*/
    );
    if (url.isEmpty())
        return;

    imageImport(url);
    // This instance will delete itself when done.
}

void Import::imageImport(const QUrl &url)
{
    Import *import = new Import;
    import->m_kimFileUrl = url;
    if (!url.isLocalFile())
        import->downloadUrl(url);
    else
        import->exec(url.path());
    // This instance will delete itself when done.
}

ImportExport::Import::Import()
    : m_tmp(nullptr)
{
}

void ImportExport::Import::downloadUrl(const QUrl &url)
{
    m_tmp = new QTemporaryFile;
    m_tmp->setFileTemplate(QString::fromLatin1("XXXXXX.kim"));
    if (!m_tmp->open()) {
        KMessageBox::error(MainWindow::Window::theMainWindow(), i18n("Unable to create temporary file"));
        delete this;
        return;
    }
    KIO::TransferJob *job = KIO::get(url);
    connect(job, &KIO::TransferJob::result, this, &Import::downloadKimJobCompleted);
    connect(job, &KIO::TransferJob::data, this, &Import::data);
}

void ImportExport::Import::downloadKimJobCompleted(KJob *job)
{
    if (job->error()) {
        job->uiDelegate()->showErrorMessage();
        delete this;
    } else {
        QString path = m_tmp->fileName();
        m_tmp->close();
        exec(path);
    }
}

void ImportExport::Import::exec(const QString &fileName)
{
    ImportDialog dialog(MainWindow::Window::theMainWindow());
    KimFileReader kimFileReader;
    if (!kimFileReader.open(fileName)) {
        delete this;
        return;
    }

    bool ok = dialog.exec(&kimFileReader, m_kimFileUrl);

    if (ok) {
        ImportHandler handler;
        handler.exec(dialog.settings(), &kimFileReader);
    }

    delete this;
}

void ImportExport::Import::data(KIO::Job *, const QByteArray &data)
{
    m_tmp->write(data);
}

ImportExport::Import::~Import()
{
    delete m_tmp;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
