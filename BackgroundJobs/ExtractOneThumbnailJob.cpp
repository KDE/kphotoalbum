// SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 - 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ExtractOneThumbnailJob.h"

#include <DB/ImageDB.h>
#include <ImageManager/ExtractOneVideoFrame.h>
#include <kpabase/ImageUtil.h>

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
    if (m_wasCanceled)
        Q_EMIT completed();
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
    Q_EMIT frameAvailable(m_fileName, m_index, image);
    Q_EMIT completed();
}

} // namespace BackgroundJobs
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ExtractOneThumbnailJob.cpp"
