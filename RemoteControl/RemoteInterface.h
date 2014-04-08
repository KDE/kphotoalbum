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
    void pixmapLoaded( const DB::FileName& fileName,
                       const QSize& size, const QSize& fullSize,
                       int angle, const QImage& image,
                       const bool loadedOK) override;


private slots:
    void handleCommand(const RemoteCommand&);

private:
    explicit RemoteInterface(QObject *parent = 0);

    void sendCategoryNames(const SearchCommand& searchInfo);
    void sendCategoryValues(const SearchCommand& search);
    void sendImageSearchResult(const SearchInfo& search);
    void requestThumbnail(const ThumbnailRequest& command);

    DB::ImageSearchInfo convert(const RemoteControl::SearchInfo&) const;
    Server* m_connection;
};

}
#endif // REMOTEINTERFACE_H
