#include "VideoThumbnailsExtractor.h"
#include <QProcess>
#include <QTextStream>
#include <QDebug>

ImageManager::VideoThumbnailsExtractor::VideoThumbnailsExtractor( const QString& fileName )
    :m_fileName(fileName)
{
    m_process = new QProcess;
    connect( m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));
    connect( m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    connect( m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));
    requestVideoLength();
}

#define STR(x) QString::fromUtf8(x)
void ImageManager::VideoThumbnailsExtractor::requestVideoLength()
{
    m_state = FetchingLength;
    QStringList arguments;
    arguments << STR("-identify") << STR("-frames") << STR("0") << STR("-vc") << STR("null") << STR("-vo") << STR("null") << STR("-ao") << STR("null") << m_fileName;
    m_process->start(STR("mplayer"), arguments);
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

    qDebug() << m_length;

}
