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
    void requestFrames();
    void requestNextFrame();
    void frameFetched();
    void thumbnailRequestCompleted();

    enum State { FetchingLength, ReadingFrames };

    QProcess* m_process;
    QString m_fileName;
    QString m_stdout;
    State m_state;
    double m_length;
    int m_frameNumber;
};

}

#endif // VIDEOTHUMBNAILSEXTRACTOR_H
