#ifndef IMAGELOADER_H
#define IMAGELOADER_H
#include <qthread.h>
class ImageManager;
class QWaitCondition;
class LoadInfo;

class ImageLoader :public QThread {
public:
    ImageLoader( QWaitCondition* sleeper );
protected:
    virtual void run();

private:
    QWaitCondition* _sleeper;
};

#endif /* IMAGELOADER_H */

