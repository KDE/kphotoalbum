#include "VideoThumbnailCycler.h"
#include <QDebug>
#include <DB/ImageInfoPtr.h>
#include <DB/ImageInfo.h>
#include <DB/Id.h>
#include <ImageManager/VideoThumbnailsExtractor.h>
#include <Utilities/Util.h>
#include <QTimer>

ThumbnailView::VideoThumbnailCycler::VideoThumbnailCycler(QObject *parent) :
    QObject(parent)
{
    m_timer = new QTimer(this);
    connect( m_timer, SIGNAL(timeout()), this, SLOT(updateThumbnail()));
}

void ThumbnailView::VideoThumbnailCycler::setActiveId(const DB::Id &id)
{
    if ( m_id == id )
        return;

    resetPreviousThumbail();
    m_id = id;
    if ( isVideo(m_id) )
        startCycle();
    else
        stopCycle();
}

void ThumbnailView::VideoThumbnailCycler::updateThumbnail()
{
    qDebug("Cycling %s", qPrintable(fileNameForId(m_id)));
    //    const QString fileName = info->fileName(DB::AbsolutePath);
//    if ( Utilities::isVideo(fileName)) {
//        qDebug() << fileName;
//        //        new ImageManager::VideoThumbnailsExtractor(fileName);
//    }
}

void ThumbnailView::VideoThumbnailCycler::resetPreviousThumbail()
{
    if ( m_id.isNull() || !isVideo(m_id) )
        return;

    qDebug("Resting %s", qPrintable(fileNameForId(m_id)));
}

bool ThumbnailView::VideoThumbnailCycler::isVideo(const DB::Id &id) const
{
    return Utilities::isVideo(fileNameForId(id));
}

QString ThumbnailView::VideoThumbnailCycler::fileNameForId(const DB::Id& id) const
{
    DB::ImageInfoPtr info = id.fetchInfo();
    return info->fileName(DB::AbsolutePath);
}

void ThumbnailView::VideoThumbnailCycler::startCycle()
{
    m_index = 0;
    m_timer->start(1000);
}

void ThumbnailView::VideoThumbnailCycler::stopCycle()
{
    m_timer->stop();
}
