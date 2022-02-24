// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "ImageNameStore.h"
#include "RemoteCommand.h"

#include <DB/search/ImageSearchInfo.h>
#include <ImageManager/ImageClientInterface.h>

#include <QHostAddress>
#include <QObject>

class QHostAddress;

namespace RemoteControl
{
class Server;

class RemoteInterface : public QObject, public ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    static RemoteInterface &instance();
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    bool requestStillNeeded(const DB::FileName &fileName);
    void listen(QHostAddress address = QHostAddress::Any);
    void stopListening();
    void connectTo(const QHostAddress &address);

private Q_SLOTS:
    void handleCommand(const RemoteCommand &);

Q_SIGNALS:
    void connected();
    void disConnected();
    void listening();
    void stoppedListening();

private:
    explicit RemoteInterface(QObject *parent = 0);

    void sendCategoryNames(const SearchRequest &searchInfo);
    void sendCategoryValues(const SearchRequest &search);
    void sendImageSearchResult(const SearchInfo &search, ImageId focusImage);
    void requestThumbnail(const ThumbnailRequest &command);
    void cancelRequest(const ThumbnailCancelRequest &command);
    void sendImageDetails(const ImageDetailsRequest &command);
    void sendHomePageImages(const StaticImageRequest &command);
    void setToken(const ToggleTokenRequest &command);
    void setupWhenConnected();
    void sendInitialDateMap();
    void sendVideo(const VideoRequest &command);

    DB::ImageSearchInfo convert(const RemoteControl::SearchInfo &) const;
    Server *m_connection = nullptr;
    QSet<DB::FileName> m_activeReuqest;
    ImageNameStore m_imageNameStore;
    class VideoServer *m_videoServer = nullptr;
};

}
#endif // REMOTEINTERFACE_H
