#include "VideoThumbnailsExtractor.h"
#include <Utilities/Process.h>
#include <QTextStream>
#include <QDebug>
#include <QImage>
#include <QDir>
#include <MainWindow/FeatureDialog.h>

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoThumbnailsExtractor::VideoThumbnailsExtractor( const QString& fileName, int videoLength )
    :m_fileName(fileName), m_length(videoLength)
{
    m_process = new Utilities::Process(this);
    m_process->setWorkingDirectory(QDir::tempPath());
    connect( m_process, SIGNAL(finished(int)), this, SLOT(frameFetched()));

    m_frameNumber = -1;
    requestNextFrame();
}

void ImageManager::VideoThumbnailsExtractor::requestNextFrame()
{
    m_frameNumber++;
    if ( m_frameNumber == 10 ) {
        thumbnailRequestCompleted();
        return;
    }

    const double offset = m_length * m_frameNumber / 10;
    QStringList arguments;
    arguments << STR("-nosound") << STR("-ss") << QString::number(offset,'g',2) << STR("-vf")
              << STR("screenshot") << STR("-frames") << STR("1") << STR("-vo") << STR("png:z=9") << m_fileName;

    m_process->start(MainWindow::FeatureDialog::mplayerBinary(), arguments);
}

void ImageManager::VideoThumbnailsExtractor::frameFetched()
{
    QImage image(QDir::tempPath() + STR("/00000001.png"));
    emit frameLoaded(m_frameNumber, image);
    requestNextFrame();
}

void ImageManager::VideoThumbnailsExtractor::thumbnailRequestCompleted()
{
    qDebug("YAY!");
}
