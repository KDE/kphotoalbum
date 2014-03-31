#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "RemoteCommand.h"
#include "DB/ImageSearchInfo.h"
#include <QObject>
#include <QHostAddress>
#include "ImageManager/ImageClientInterface.h"


namespace RemoteControl
{
class Server;

class RemoteInterface : public QObject, ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    static RemoteInterface& instance();
    void sendImageCount(int count);
    void pixmapLoaded( const DB::FileName& fileName,
                       const QSize& size, const QSize& fullSize,
                       int angle, const QImage& image,
                       const bool loadedOK) override;


private slots:
    void handleCommand(const RemoteCommand&);

private:
    void sendCategoryNames(const RequestCategoryInfo& searchInfo);
    void sendCategoryValues(const RequestCategoryInfo& search);
    void sendImageSearchResult(const SearchInfo& search);
    void requestThumbnail(const ThumbnailRequest& command);

    explicit RemoteInterface(QObject *parent = 0);
    DB::ImageSearchInfo convert(const RemoteControl::SearchInfo&) const;
    Server* m_connection;
};

}
#endif // REMOTEINTERFACE_H
