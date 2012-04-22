#ifndef VIDEOTHUMBNAILSEXTRACTOR_H
#define VIDEOTHUMBNAILSEXTRACTOR_H

class QProcess;
#include <QObject>

namespace ImageManager
{

class VideoThumbnailsExtractor :public QObject
{
Q_OBJECT

public:
    VideoThumbnailsExtractor( const QString& fileName );

private slots:
    void readStandardError();
    void readStandardOutput();
    void processEnded();
    void extractVideoLength();

private:
    void requestVideoLength();

    enum State { FetchingLength, ReadingFrames };

    QProcess* m_process;
    QString m_fileName;
    QString m_stdout;
    State m_state;
    double m_length;
};

}

#endif // VIDEOTHUMBNAILSEXTRACTOR_H
