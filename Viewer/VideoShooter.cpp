#include "VideoShooter.h"
#include <QTimer>
#include "ViewerWidget.h"
#include "VideoDisplay.h"
#include "InfoBox.h"
#include <QApplication>
#include <ImageManager/VideoManager.h>
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
    const QString& fileName = m_info->fileName(DB::AbsolutePath);
    ImageManager::ThumbnailCache::instance()->removeThumbnail( fileName );
    ImageManager::VideoManager::saveFullScaleFrame(fileName, image);

    // Show the infobox again
    if ( m_infoboxVisible )
        m_viewer->_infoBox->show();

    // Restart the video
    if ( m_wasPlaying )
        m_viewer->_videoDisplay->playPause();

    qApp->restoreOverrideCursor();
}
