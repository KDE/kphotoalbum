#ifndef VIDEOTHUMBNAILSEXTRACTOR_H
#define VIDEOTHUMBNAILSEXTRACTOR_H

#include <QObject>
class QProcess;
class QImage;

namespace Utilities { class Process; }

namespace ImageManager
{

class VideoThumbnailsExtractor :public QObject
{
Q_OBJECT

public:
    VideoThumbnailsExtractor( const QString& fileName, int videoLength );

private slots:
    void frameFetched();

signals:
    void frameLoaded(int index, const QImage& image );

private:
    void requestNextFrame();
    void thumbnailRequestCompleted();

    Utilities::Process* m_process;
    QString m_fileName;
    double m_length;
    int m_frameNumber;
};

}

#endif // VIDEOTHUMBNAILSEXTRACTOR_H
