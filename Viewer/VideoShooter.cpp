/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "VideoShooter.h"

#include "InfoBox.h"
#include "VideoDisplay.h"
#include "ViewerWidget.h"

#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <DB/ImageInfo.h>
#include <MainWindow/Window.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <QApplication>
#include <QTimer>

/**
  \class Viewer::VideoShooter
  \brief Utility class for making screenshots from a video

  While Phonon has an API for implementing a screenshot, it unfortunately doesn't work.
  We therefore need to make a screenshot using QPixmap::grabWindow (grabWidget doesn't work either).
  As grabWindow takes the pixels directly from the window, we need to make sure that it isn't covered with the infobox or the context menu.
  This class takes care of that, namely waiting for the context menu to disapear, hide the infobox, stop the video, and then shoot the snapshot
 */

Viewer::VideoShooter *Viewer::VideoShooter::s_instance = nullptr;

Viewer::VideoShooter::VideoShooter()
{
}

void Viewer::VideoShooter::go(const DB::ImageInfoPtr &info, Viewer::ViewerWidget *viewer)
{
    if (!s_instance)
        s_instance = new VideoShooter;

    s_instance->start(info, viewer);
}

void Viewer::VideoShooter::start(const DB::ImageInfoPtr &info, ViewerWidget *viewer)
{
    qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    m_info = info;
    m_viewer = viewer;

    // Hide the info box
    m_infoboxVisible = m_viewer->m_infoBox->isVisible();
    if (m_infoboxVisible)
        m_viewer->m_infoBox->hide();

    // Stop playback
    m_wasPlaying = !m_viewer->m_videoDisplay->isPaused();
    if (m_wasPlaying)
        m_viewer->m_videoDisplay->playPause();

    // Wait a bit for the context menu to disapear
    QTimer::singleShot(200, this, &VideoShooter::doShoot);
}

void Viewer::VideoShooter::doShoot()
{
    // Make the screenshot and save it
    const QImage image = m_viewer->m_videoDisplay->screenShoot();
    const DB::FileName fileName = m_info->fileName();
    MainWindow::Window::theMainWindow()->thumbnailCache()->removeThumbnail(fileName);
    BackgroundJobs::HandleVideoThumbnailRequestJob::saveFullScaleFrame(fileName, image);

    // Show the infobox again
    if (m_infoboxVisible)
        m_viewer->m_infoBox->show();

    // Restart the video
    if (m_wasPlaying)
        m_viewer->m_videoDisplay->playPause();

    qApp->restoreOverrideCursor();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
