/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ExtractOneThumbnailJob.h"

#include "HandleVideoThumbnailRequestJob.h"

#include <DB/ImageDB.h>
#include <ImageManager/ExtractOneVideoFrame.h>
#include <Utilities/ImageUtil.h>

#include <KLocalizedString>
#include <QFile>
#include <QImage>
#include <QPainter>

namespace BackgroundJobs
{

ExtractOneThumbnailJob::ExtractOneThumbnailJob(const DB::FileName &fileName, int index, BackgroundTaskManager::Priority priority)
    : JobInterface(priority)
    , m_fileName(fileName)
    , m_index(index)
    , m_wasCanceled(false)
{
    Q_ASSERT(index >= 0 && index <= 9);
}

void ExtractOneThumbnailJob::execute()
{
    if (m_wasCanceled || frameName().exists())
        emit completed();
    else {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info(m_fileName);
        const int length = info->videoLength();
        ImageManager::ExtractOneVideoFrame::extract(m_fileName, length * m_index / 10.0, this, SLOT(frameLoaded(QImage)));
    }
}

QString ExtractOneThumbnailJob::title() const
{
    return i18n("Extracting Thumbnail");
}

QString ExtractOneThumbnailJob::details() const
{
    return QString::fromLatin1("%1 #%2").arg(m_fileName.relative()).arg(m_index);
}

int ExtractOneThumbnailJob::index() const
{
    return m_index;
}

void ExtractOneThumbnailJob::cancel()
{
    m_wasCanceled = true;
}

void ExtractOneThumbnailJob::frameLoaded(const QImage &image)
{
    if (!image.isNull()) {
#if 0
        QImage img = image;
        {
            QPainter painter(&img);
            QFont fnt;
            fnt.setPointSize(24);
            painter.setFont(fnt);
            painter.drawText(QPoint(100,100),QString::number(m_index));
        }
#endif
        Utilities::saveImage(frameName(), image, "JPEG");
    } else {
        // Create empty file to avoid that we recheck at next start up.
        QFile file(frameName().absolute());
        file.open(QFile::WriteOnly);
        file.close();
    }
    emit completed();
}

DB::FileName ExtractOneThumbnailJob::frameName() const
{
    return BackgroundJobs::HandleVideoThumbnailRequestJob::frameName(m_fileName, m_index);
}

} // namespace BackgroundJobs
// vi:expandtab:tabstop=4 shiftwidth=4:
