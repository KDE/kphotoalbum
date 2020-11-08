/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ImageRow.h"

#include "ImportDialog.h"
#include "KimFileReader.h"
#include "Logging.h"
#include "MiniViewer.h"

#include <MainWindow/Window.h>

#include <KIO/StoredTransferJob>
#include <KJobUiDelegate>
#include <KJobWidgets>
#include <QCheckBox>
#include <QImage>
#include <memory>

using namespace ImportExport;

ImageRow::ImageRow(DB::ImageInfoPtr info, ImportDialog *import, KimFileReader *kimFileReader, QWidget *parent)
    : QObject(parent)
    , m_info(info)
    , m_import(import)
    , m_kimFileReader(kimFileReader)
{
    m_checkbox = new QCheckBox(QString(), parent);
    m_checkbox->setChecked(true);
}

void ImageRow::showImage()
{
    if (m_import->m_externalSource) {
        QUrl src1 = m_import->m_kimFile;
        QUrl src2 = m_import->m_baseUrl;
        for (int i = 0; i < 2; ++i) {
            // First try next to the .kim file, then the external URL
            QUrl src = src1;
            if (i == 1)
                src = src2;
            src = src.adjusted(QUrl::RemoveFilename);
            src.setPath(src.path() + m_info->fileName().relative());
            QString tmpFile;

            std::unique_ptr<KIO::StoredTransferJob> downloadJob { KIO::storedGet(src) };
            KJobWidgets::setWindow(downloadJob.get(), MainWindow::Window::theMainWindow());

            if (downloadJob->exec()) {
                QImage img;
                if (img.loadFromData(downloadJob->data())) {
                    MiniViewer::show(img, m_info, static_cast<QWidget *>(parent()));
                    break;
                } else {
                    qCWarning(ImportExportLog) << "Could not load image data for" << src.toDisplayString();
                }
            }
        }
    } else {
        QImage img = QImage::fromData(m_kimFileReader->loadImage(m_info->fileName().relative()));
        MiniViewer::show(img, m_info, static_cast<QWidget *>(parent()));
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
