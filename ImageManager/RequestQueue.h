#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#include "StopAction.h"
#include <qvaluelist.h>
#include <Utilities/Set.h>
#include <qobject.h>

namespace ImageManager
{
class ImageRequest;
class ImageClient;

class RequestQueue :public QObject
{
    Q_OBJECT

public:
    RequestQueue();

    void addRequest( ImageRequest* request );
    ImageRequest* popNext();
    void cancelRequests( ImageClient* client, StopAction action );
    bool isRequestStillValid( ImageRequest* request );
    void removeRequest( ImageRequest* );

protected slots:
    void print();

private:
    QValueList<ImageRequest*> _pendingRequests;
    Set<ImageRequest*> _activeRequests;
};

}

#endif /* REQUESTQUEUE_H */

