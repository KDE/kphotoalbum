#ifndef CANCELEVENT_H
#define CANCELEVENT_H

#include <QEvent>

namespace ImageManager {

class ImageRequest;

const int CANCELEVENTID = 1002;

class CancelEvent : public QEvent
{
public:
    CancelEvent( ImageRequest* request );
    ~CancelEvent();
    ImageRequest* request() const;

private:
    ImageRequest* m_request;
};

}
#endif // CANCELEVENT_H
