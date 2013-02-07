/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "VideoShooter.h"
#include <QTimer>
#include "ViewerWidget.h"
#include "VideoDisplay.h"
#include "InfoBox.h"
#include <QApplication>
#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <DB/ImageInfo.h>
#include <ImageManager/ThumbnailCache.h>

/**
  \class Viewer::VideoShooter
  \brief Utility class for making screenshots from a video

  While Phonon has an API for implementing a screenshot, it unfortunately doesn't work.
  We therefore need to make a screenshot using QPixmap::grabWindow (grabWidget doesn't work either).
  As grabWindow takes the pixels directly from the window, we need to make sure that it isn't covered with the infobox or the context menu.
  This class takes care of that, namely waiting for the context menu to disapear, hide the infobox, stop the video, and then shoot the snapshot
 */

Viewer::VideoShooter* Viewer::VideoShooter::m_instance = 0;

Viewer::VideoShooter::VideoShooter()
{
}

void Viewer::VideoShooter::go(const DB::ImageInfoPtr& info, Viewer::ViewerWidget *viewer)
{
    if ( !m_instance)
        m_instance = new VideoShooter;

    m_instance->start(info, viewer);
}

void Viewer::VideoShooter::start(const DB::ImageInfoPtr& info, ViewerWidget* viewer)
{
    qApp->setOverrideCursor( QCursor( Qt::BusyCursor ) );
    m_info = info;
    m_viewer = viewer;

    // Hide the info box
    m_infoboxVisible = m_viewer->_infoBox->isVisible();
    if ( m_infoboxVisible )
        m_viewer->_infoBox->hide();

    // Stop playback
    m_wasPlaying = !m_viewer->_videoDisplay->isPaused();
    if ( m_wasPlaying )
        m_viewer->_videoDisplay->playPause();

    // Wait a bit for the context menu to disapear
    QTimer::singleShot(200, this, SLOT(doShoot()));
}

void Viewer::VideoShooter::doShoot()
{
    // Make the screenshot and save it
    const QImage image = m_viewer->_videoDisplay->screenShoot();
    const DB::FileName fileName = m_info->fileName();
    ImageManager::ThumbnailCache::instance()->removeThumbnail( fileName );
    BackgroundJobs::HandleVideoThumbnailRequestJob::saveFullScaleFrame(fileName, image);

    // Show the infobox again
    if ( m_infoboxVisible )
        m_viewer->_infoBox->show();

    // Restart the video
    if ( m_wasPlaying )
        m_viewer->_videoDisplay->playPause();

    qApp->restoreOverrideCursor();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
