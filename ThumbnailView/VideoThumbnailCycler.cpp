// SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2012-2025 The KPhotoAlbum Development Team

// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "VideoThumbnailCycler.h"

#include "CellGeometry.h"
#include "ThumbnailModel.h"

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <ImageManager/VideoThumbnails.h>
#include <kpabase/FileExtensions.h>

#include <QTimer>

ThumbnailView::VideoThumbnailCycler *ThumbnailView::VideoThumbnailCycler::s_instance = nullptr;

ThumbnailView::VideoThumbnailCycler::VideoThumbnailCycler(ThumbnailModel *model, QObject *parent)
    : QObject(parent)
    , m_thumbnails(new ImageManager::VideoThumbnails(this))
    , m_model(model)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, m_thumbnails, &ImageManager::VideoThumbnails::requestNext);
    connect(m_thumbnails, &ImageManager::VideoThumbnails::frameLoaded, this, &VideoThumbnailCycler::gotFrame);
    Q_ASSERT(!s_instance);
    s_instance = this;
}

ThumbnailView::VideoThumbnailCycler *ThumbnailView::VideoThumbnailCycler::instance()
{
    Q_ASSERT(s_instance);
    return s_instance;
}

void ThumbnailView::VideoThumbnailCycler::setActive(const DB::FileName &fileName)
{
    if (m_fileName == fileName)
        return;

    stopCycle();

    m_fileName = fileName;
    if (!m_fileName.isNull() && isVideo(m_fileName))
        startCycle();
}

void ThumbnailView::VideoThumbnailCycler::gotFrame(const QImage &image)
{
    QImage img = image.scaled(ThumbnailView::CellGeometry::preferredIconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (!m_model->setOverrideImage(m_fileName, QPixmap::fromImage(img))) {
        // The image is no longer in the current view.
        stopCycle();
    }
}

void ThumbnailView::VideoThumbnailCycler::resetPreviousThumbail()
{
    if (m_fileName.isNull() || !isVideo(m_fileName))
        return;

    m_model->setOverrideImage(m_fileName, QPixmap());
}

bool ThumbnailView::VideoThumbnailCycler::isVideo(const DB::FileName &fileName) const
{
    if (!fileName.isNull())
        return KPABase::isVideo(fileName);
    else
        return false;
}

void ThumbnailView::VideoThumbnailCycler::startCycle()
{
    m_thumbnails->setVideoFile(m_fileName);
    m_timer->start(500);
    m_thumbnails->requestNext(); // We want it to cycle right away.
}

void ThumbnailView::VideoThumbnailCycler::stopCycle()
{
    resetPreviousThumbail();
    m_fileName = DB::FileName();
    m_timer->stop();
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VideoThumbnailCycler.cpp"
