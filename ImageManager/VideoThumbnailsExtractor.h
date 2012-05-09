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
    static QString frameName(const QString& videoName, int frameNumber );

private slots:
    void frameFetched();

signals:
    void frameLoaded(int index, const QImage& image );
    void completed();

private:
    void requestNextFrame();

    Utilities::Process* m_process;
    QString m_fileName;
    double m_length;
    int m_frameNumber;
};

}

#endif // VIDEOTHUMBNAILSEXTRACTOR_H
