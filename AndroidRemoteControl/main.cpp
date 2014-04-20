#include <QApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

#include "RemoteInterface.h"
#include "RemoteImage.h"
#include "MyImage.h"
#include "Settings.h"
#include "ScreenInfo.h"
#include "ImageStore.h"
#include "ImageDetails.h"

using namespace RemoteControl;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("KDE");
    QCoreApplication::setOrganizationDomain("kde.org");
    QCoreApplication::setApplicationName("KPhotoAlbum");

    QQuickView viewer;
    ScreenInfo::instance().setScreen(viewer.screen());
    QObject::connect(viewer.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    qmlRegisterType<RemoteImage>("KPhotoAlbum", 1, 0, "RemoteImage");
    qmlRegisterType<MyImage>("KPhotoAlbum", 1, 0, "MyImage");
    qmlRegisterUncreatableType<Types>("KPhotoAlbum", 1, 0, "Enums","Dont create instances of this class");

    QQmlContext* rootContext = viewer.engine()->rootContext();
    rootContext->setContextProperty(QStringLiteral("_remoteInterface"), &RemoteInterface::instance());
    rootContext->setContextProperty(QStringLiteral("_settings"), &Settings::instance());
    rootContext->setContextProperty(QStringLiteral("_screenInfo"), &ScreenInfo::instance());
    rootContext->setContextProperty(QStringLiteral("_imageDetails"), &ImageDetails::instance());

    viewer.setSource(QStringLiteral("qrc:/qml/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);

    viewer.resize(1024,768);
    viewer.show();

    // Create the store on the GUI thread
    (void) ImageStore::instance();

    return app.exec();
}
