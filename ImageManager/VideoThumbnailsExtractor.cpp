#include "VideoThumbnailsExtractor.h"
#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QImage>
#include <QDir>
#include <MainWindow/FeatureDialog.h>

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoThumbnailsExtractor::VideoThumbnailsExtractor( const QString& fileName )
    :m_fileName(fileName)
{
    m_process = new QProcess;
    m_process->setWorkingDirectory(QDir::tempPath());
    connect( m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));
    connect( m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    connect( m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));
    requestVideoLength();
}

void ImageManager::VideoThumbnailsExtractor::requestVideoLength()
{
    m_state = FetchingLength;
    QStringList arguments;

    // PENDING(blackie) Handle situation where the binary is not there!
    arguments << STR("-identify") << STR("-frames") << STR("0") << STR("-vc") << STR("null") << STR("-vo") << STR("null") << STR("-ao") << STR("null") << m_fileName;
    m_process->start(MainWindow::FeatureDialog::mplayerBinary(), arguments);
}

void ImageManager::VideoThumbnailsExtractor::requestFrames()
{
    m_state = ReadingFrames;
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
    arguments << STR("-nosound") << STR("-ss") << QString::number(offset,'g',2) << STR("-vf") << STR("screenshot") << STR("-frames") << STR("1") << STR("-vo") << STR("png:z=9") << m_fileName;

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

void ImageManager::VideoThumbnailsExtractor::readStandardError()
{
    m_process->setReadChannel(QProcess::StandardError);
    QTextStream stream(m_process);
    QString output = stream.readAll();
    qWarning() << output;
}

void ImageManager::VideoThumbnailsExtractor::readStandardOutput()
{
    m_process->setReadChannel(QProcess::StandardOutput);
    QTextStream stream(m_process);
    QString output = stream.readAll();
    m_stdout.append(output);
}

void ImageManager::VideoThumbnailsExtractor::processEnded()
{
    if ( m_state == FetchingLength )
        extractVideoLength();
    else
        frameFetched();
}

void ImageManager::VideoThumbnailsExtractor::extractVideoLength()
{
    QStringList list = m_stdout.split(QChar::fromLatin1('\n'));
    list = list.filter(STR("ID_LENGTH="));
    if ( list.count() != 1 ) {
        qWarning("Unknown list length: %d", list.count());
        return;
    }

    QString str = list[0];
    QRegExp regexp(STR("ID_LENGTH=([0-9.]+)"));
    bool ok = regexp.exactMatch(str);
    if ( !ok ) {
        qWarning("Unable to match regexp for string: %s", qPrintable(str));
        return;
    }

    const QString cap = regexp.cap(1);

    m_length = cap.toDouble(&ok);
    if ( !ok ) {
        qWarning("Unable to convert string \"%s\"to integer", qPrintable(cap));
        return;
    }

    if ( m_length == 0 ) {
        qWarning("Length returned was 0!");
        return;
    }

    requestFrames();
}
