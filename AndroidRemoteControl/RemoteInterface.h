#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "CategoryModel.h"
#include "RemoteCommand.h"

#include <QObject>
#include <QMap>
#include <QImage>
#include <QStringList>

class QTcpSocket;

namespace RemoteControl {

class RemoteConnection;

class RemoteInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool connected READ isConnected NOTIFY connectionChanged )
    Q_PROPERTY(int imageCount MEMBER m_imageCount NOTIFY imageCountChanged)
    Q_PROPERTY(RemoteControl::CategoryModel* categories MEMBER m_categories NOTIFY categoriesChanged)
    Q_PROPERTY(QImage home MEMBER m_homeImage NOTIFY homeImageChanged)

public:
    static RemoteInterface& instance();
    bool isConnected() const;
    QImage image(int index) const;

public slots:
    void previousSlide();
    void nextSlide();

signals:
    void connectionChanged();
    void imageUpdated(int index);
    void imageCountChanged();
    void categoriesChanged();
    void homeImageChanged();

private slots:
    void handleCommand(const RemoteCommand&);
    void updateImage(const ImageUpdateCommand&);
    void updateImageCount(const ImageCountUpdateCommand&);
    void updateCategoryList(const CategoryListCommand&);

private:
    RemoteInterface();

    RemoteConnection* m_connection = nullptr;
    int m_imageCount = 0;
    QMap<int,QImage> m_imageMap;
    CategoryModel* m_categories;
    QImage m_homeImage;
};

}

#endif // REMOTEINTERFACE_H
