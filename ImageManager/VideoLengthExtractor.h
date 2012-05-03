#ifndef VIDEOLENGTHEXTRACTOR_H
#define VIDEOLENGTHEXTRACTOR_H

#include <QObject>

namespace Utilities { class Process; }

namespace ImageManager {

class VideoLengthExtractor : public QObject
{
    Q_OBJECT
public:
    explicit VideoLengthExtractor(QObject *parent = 0);
    void extract(const QString& fileName );
    
signals:
    void lengthFound(int length);
    void unableToDetermineLength();

private slots:
    void processEnded();

private:
    Utilities::Process* m_process;
};

}

#endif // VIDEOLENGTHEXTRACTOR_H
