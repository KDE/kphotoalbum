#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "CategoryModel.h"
#include "RemoteCommand.h"
#include "SearchInfo.h"
#include "CategoryItemsModel.h"

#include <QObject>
#include <QMap>
#include <QImage>
#include <QStringList>
#include "History.h"

class QTcpSocket;

namespace RemoteControl {

class Client;

class RemoteInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool connected READ isConnected NOTIFY connectionChanged )
    Q_PROPERTY(RemoteControl::CategoryModel* categories MEMBER m_categories NOTIFY categoriesChanged)
    Q_PROPERTY(RemoteControl::CategoryItemsModel* categoryItemsModel MEMBER m_categoryItems NOTIFY categoryItemChanged)
    Q_PROPERTY(QImage home MEMBER m_homeImage NOTIFY homeImageChanged)
    Q_PROPERTY(QImage kphotoalbum MEMBER m_kphotoalbumImage NOTIFY kphotoalbumImageChange)
    Q_PROPERTY(QString currentPage MEMBER m_currentPage NOTIFY currentPageChanged) //PENDING(blackie) convert into an enum
    Q_PROPERTY(QStringList thumbnails MEMBER m_thumbnails NOTIFY thumbnailsChanged)


public:
    static RemoteInterface& instance();
    bool isConnected() const;
    void sendCommand(const RemoteCommand& command);

public slots:
    void goHome();
    void goBack();
    void goForward();
    void selectCategory(const QString& category);
    void selectCategoryValue(const QString& value);
    void showThumbnails();
    void showImage(const QString& fileName);

signals:
    void connectionChanged();
    void categoriesChanged();
    void homeImageChanged();
    void kphotoalbumImageChange();    
    void categoryItemChanged();
    void currentPageChanged();
    void thumbnailsChanged();
    void jumpToImage(int index);

public:
    void setCurrentView(const QString& image);

private slots:
    void requestInitialData();
    void handleCommand(const RemoteCommand&);
    void updateImage(const ImageUpdateCommand&);
    void updateCategoryList(const CategoryListCommand&);
    void gotCategoryItems(const CategoryItemListCommand&);
    void gotImageSearchResult(const ImageSearchResult&);

private:
    RemoteInterface();
    friend class Action;
    void setCurrentPage(const QString& page);

    Client* m_connection = nullptr;
    CategoryModel* m_categories;
    CategoryItemsModel* m_categoryItems;
    QImage m_homeImage;
    QImage m_kphotoalbumImage;
    SearchInfo m_search;
    RemoteControl::CategoryItemsModel* m_categotyItems;
    QString m_currentPage = QStringLiteral("Unconnected");
    QStringList m_thumbnails;
    History m_history;
};

}

#endif // REMOTEINTERFACE_H
