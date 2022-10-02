// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "CategoryModel.h"
#include "DiscoveryModel.h"
#include "RemoteCommand.h"
#include "SearchInfo.h"
#include "ThumbnailModel.h"

#include "History.h"
#include "Types.h"
#include <QImage>
#include <QMap>
#include <QObject>
#include <QStringList>

class QTcpSocket;

namespace RemoteControl
{

class Client;

class RemoteInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(RemoteControl::CategoryModel *categories MEMBER m_categories NOTIFY categoriesChanged)
    Q_PROPERTY(ThumbnailModel *categoryItems MEMBER m_categoryItems NOTIFY categoryItemsChanged)
    Q_PROPERTY(QImage home MEMBER m_homeImage NOTIFY homeImageChanged)
    Q_PROPERTY(QImage kphotoalbum MEMBER m_kphotoalbumImage NOTIFY kphotoalbumImageChange)
    Q_PROPERTY(QImage discoveryImage MEMBER m_discoveryImage NOTIFY discoveryImageChanged)
    Q_PROPERTY(RemoteControl::Types::Page currentPage MEMBER m_currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(ThumbnailModel *thumbnailModel MEMBER m_thumbnailModel NOTIFY thumbnailModelChanged)
    Q_PROPERTY(QStringList listCategoryValues MEMBER m_listCategoryValues NOTIFY listCategoryValuesChanged)
    Q_PROPERTY(DiscoveryModel *discoveryModel MEMBER m_discoveryModel NOTIFY discoveryModelChanged)
    Q_PROPERTY(ThumbnailModel *activeThumbnailModel MEMBER m_activeThumbnailModel NOTIFY activeThumbnailModelChanged)
    Q_PROPERTY(QString networkAddress READ networkAddress NOTIFY networkAddressChanged)
    Q_PROPERTY(QStringList tokens READ tokens NOTIFY tokensChanged)

public:
    static RemoteInterface &instance();
    bool isConnected() const;
    void sendCommand(const RemoteCommand &command);
    QString currentCategory() const;
    QImage discoveryImage() const;

    enum class ModelType { Thumbnail,
                           Discovery };
    void setActiveThumbnailModel(ModelType);

public Q_SLOTS:
    void goHome();
    void goBack();
    void goForward();
    void selectCategory(const QString &category, int /*CategoryViewType*/ type);
    void selectCategoryValue(const QString &value);
    void showThumbnails();
    void showImage(int imageId);
    void requestDetails(int imageId);
    void activateSearch(const QString &search);
    void doDiscovery();
    void showOverviewPage();
    void setToken(int imageId, const QString &token);
    void removeToken(int imageId, const QString &token);
    void rerequestOverviewPageData();
    void pushAwayFromStartupState();

Q_SIGNALS:
    void connectionChanged();
    void categoriesChanged();
    void homeImageChanged();
    void kphotoalbumImageChange();
    void categoryItemsChanged();
    void currentPageChanged();
    void thumbnailModelChanged();
    void jumpToImage(int index);
    void listCategoryValuesChanged();

    void discoveryImageChanged();
    void discoveryModelChanged();

    void activeThumbnailModelChanged();
    void networkAddressChanged();
    void tokensChanged();

public:
    void setCurrentView(int imageId);
    QString networkAddress() const;

    QStringList tokens() const;

private Q_SLOTS:
    void requestInitialData();
    void handleCommand(const RemoteCommand &);
    void updateImage(const ThumbnailResult &);
    void updateCategoryList(const CategoryListResult &);
    void gotSearchResult(const SearchResult &);
    void requestHomePageImages();
    void gotDisconnected();

private:
    RemoteInterface();
    friend class Action;
    void setCurrentPage(Page page);
    void setListCategoryValues(const QStringList &values);
    void setHomePageImages(const StaticImageResult &command);

    Client *m_connection = nullptr;
    CategoryModel *m_categories;
    QImage m_homeImage;
    QImage m_kphotoalbumImage;
    SearchInfo m_search;
    ThumbnailModel *m_categoryItems;
    RemoteControl::Page m_currentPage = RemoteControl::Page::Startup;
    ThumbnailModel *m_thumbnailModel;
    History m_history;
    QStringList m_listCategoryValues;
    QImage m_discoveryImage;
    DiscoveryModel *m_discoveryModel;
    ThumbnailModel *m_activeThumbnailModel = nullptr;
};

}

#endif // REMOTEINTERFACE_H
