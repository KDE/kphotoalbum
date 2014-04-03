#include <QApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

#include "RemoteInterface.h"
#include "RemoteImage.h"
#include "MyImage.h"
#include "Settings.h"
#include "ScreenInfo.h"

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
    QQmlContext* rootContext = viewer.engine()->rootContext();
    rootContext->setContextProperty(QStringLiteral("_remoteInterface"), &RemoteInterface::instance());
    rootContext->setContextProperty(QStringLiteral("_settings"), &Settings::instance());
    rootContext->setContextProperty(QStringLiteral("_screenInfo"), &ScreenInfo::instance());

    viewer.setSource(QStringLiteral("qrc:/qml/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);

    viewer.resize(1024,768);
    viewer.show();

    qDebug() << ScreenInfo::instance().pixelForSizeInMM(100,100);

    return app.exec();
}
