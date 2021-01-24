// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "KimFileReader.h"

#include <Utilities/VideoUtil.h>
#include <kpabase/FileNameUtil.h>

#include <KLocalizedString>
#include <QFileInfo>
#include <kmessagebox.h>
#include <kzip.h>

ImportExport::KimFileReader::KimFileReader()
    : m_zip(nullptr)
{
}

bool ImportExport::KimFileReader::open(const QString &fileName)
{
    m_fileName = fileName;
    m_zip = new KZip(fileName);
    if (!m_zip->open(QIODevice::ReadOnly)) {
        KMessageBox::error(nullptr, i18n("Unable to open '%1' for reading.", fileName), i18n("Error Importing Data"));
        delete m_zip;
        m_zip = nullptr;
        return false;
    }

    m_dir = m_zip->directory();
    if (m_dir == nullptr) {
        KMessageBox::error(nullptr, i18n("Error reading directory contents of file %1; it is likely that the file is broken.", fileName));
        delete m_zip;
        m_zip = nullptr;
        return false;
    }

    return true;
}

QByteArray ImportExport::KimFileReader::indexXML()
{
    const KArchiveEntry *indexxml = m_dir->entry(QString::fromLatin1("index.xml"));
    if (indexxml == nullptr || !indexxml->isFile()) {
        KMessageBox::error(nullptr, i18n("Error reading index.xml file from %1; it is likely that the file is broken.", m_fileName));
        return QByteArray();
    }

    const KArchiveFile *file = static_cast<const KArchiveFile *>(indexxml);
    return file->data();
}

ImportExport::KimFileReader::~KimFileReader()
{
    delete m_zip;
}

QPixmap ImportExport::KimFileReader::loadThumbnail(QString fileName)
{
    const KArchiveEntry *thumbnails = m_dir->entry(QString::fromLatin1("Thumbnails"));
    if (!thumbnails)
        return QPixmap();

    if (!thumbnails->isDirectory()) {
        KMessageBox::error(nullptr, i18n("Thumbnail item in export file was not a directory, this indicates that the file is broken."));
        return QPixmap();
    }

    const KArchiveDirectory *thumbnailDir = static_cast<const KArchiveDirectory *>(thumbnails);

    const auto fileInfo = QFileInfo(fileName);
    const QString ext = Utilities::isVideo(DB::FileName::fromRelativePath(fileName)) ? QString::fromLatin1("jpg") : fileInfo.completeSuffix();
    fileName = QString::fromLatin1("%1.%2").arg(fileInfo.baseName()).arg(ext);
    const KArchiveEntry *fileEntry = thumbnailDir->entry(fileName);
    if (fileEntry == nullptr || !fileEntry->isFile()) {
        KMessageBox::error(nullptr, i18n("No thumbnail existed in export file for %1", fileName));
        return QPixmap();
    }

    const KArchiveFile *file = static_cast<const KArchiveFile *>(fileEntry);
    const QByteArray data = file->data();
    QPixmap pixmap;
    pixmap.loadFromData(data);
    return pixmap;
}

QByteArray ImportExport::KimFileReader::loadImage(const QString &fileName)
{
    const KArchiveEntry *images = m_dir->entry(QString::fromLatin1("Images"));
    if (!images) {
        KMessageBox::error(nullptr, i18n("export file did not contain a Images subdirectory, this indicates that the file is broken"));
        return QByteArray();
    }

    if (!images->isDirectory()) {
        KMessageBox::error(nullptr, i18n("Images item in export file was not a directory, this indicates that the file is broken"));
        return QByteArray();
    }

    const KArchiveDirectory *imagesDir = static_cast<const KArchiveDirectory *>(images);

    const KArchiveEntry *fileEntry = imagesDir->entry(fileName);
    if (fileEntry == nullptr || !fileEntry->isFile()) {
        KMessageBox::error(nullptr, i18n("No image existed in export file for %1", fileName));
        return QByteArray();
    }

    const KArchiveFile *file = static_cast<const KArchiveFile *>(fileEntry);
    QByteArray data = file->data();
    return data;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
