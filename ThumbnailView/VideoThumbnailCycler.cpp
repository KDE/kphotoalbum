/* Copyright 2012  Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "VideoThumbnailCycler.h"
#include <DB/ImageInfoPtr.h>
#include <DB/ImageInfo.h>
#include <Utilities/VideoUtil.h>
#include <QTimer>
#include <ImageManager/VideoThumbnails.h>
#include "ThumbnailModel.h"
#include <ThumbnailView/CellGeometry.h>

ThumbnailView::VideoThumbnailCycler* ThumbnailView::VideoThumbnailCycler::s_instance = nullptr;

ThumbnailView::VideoThumbnailCycler::VideoThumbnailCycler(ThumbnailModel* model, QObject *parent) :
    QObject(parent), m_thumbnails( new ImageManager::VideoThumbnails(this)), m_model(model)
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
    if ( m_fileName == fileName )
        return;

    stopCycle();

    m_fileName = fileName;
    if ( !m_fileName.isNull() && isVideo(m_fileName))
        startCycle();
}


void ThumbnailView::VideoThumbnailCycler::gotFrame(const QImage &image)
{
    QImage img = image.scaled(ThumbnailView::CellGeometry::preferredIconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_model->setOverrideImage(m_fileName, QPixmap::fromImage(img));
}

void ThumbnailView::VideoThumbnailCycler::resetPreviousThumbail()
{
    if ( m_fileName.isNull() || !isVideo(m_fileName) )
        return;

    m_model->setOverrideImage(m_fileName,QPixmap());
}

bool ThumbnailView::VideoThumbnailCycler::isVideo(const DB::FileName &fileName) const
{
    if ( !fileName.isNull() )
        return Utilities::isVideo(fileName);
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
