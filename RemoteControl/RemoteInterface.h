#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "ImageNameStore.h"
#include "RemoteCommand.h"
#include "DB/ImageSearchInfo.h"
#include <QObject>
#include <QHostAddress>
#include "ImageManager/ImageClientInterface.h"


namespace RemoteControl
{
class Server;

class RemoteInterface : public QObject, public ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    static RemoteInterface& instance();
    void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image);
    bool requestStillNeeded(const DB::FileName& fileName);


private slots:
    void handleCommand(const RemoteCommand&);

private:
    explicit RemoteInterface(QObject *parent = 0);

    void sendCategoryNames(const SearchCommand& searchInfo);
    void sendCategoryValues(const SearchCommand& search);
    void sendImageSearchResult(const SearchInfo& search);
    void requestThumbnail(const ThumbnailRequest& command);
    void cancelRequest(const CancelRequestCommand& command);
    void sendImageDetails(const RequestDetails& command);

    DB::ImageSearchInfo convert(const RemoteControl::SearchInfo&) const;
    Server* m_connection;
    QSet<DB::FileName> m_activeReuqest;
    ImageNameStore m_imageNameStore;
};

}
#endif // REMOTEINTERFACE_H
