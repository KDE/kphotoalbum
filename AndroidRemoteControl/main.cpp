#include <QApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

#include "RemoteInterface.h"
#include "RemoteImage.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("KDAB");
    QCoreApplication::setOrganizationDomain("kdab.com");
    QCoreApplication::setApplicationName("RemoteControl");

    QQuickView viewer;
    QObject::connect(viewer.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    qmlRegisterType<RemoteControl::RemoteImage>("SlideViewer", 1, 0, "RemoteImage");
    viewer.engine()->rootContext()->setContextProperty("_remoteInterface", &RemoteControl::RemoteInterface::instance());

    viewer.setSource(QStringLiteral("qrc:/qml/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);

    viewer.resize(1024,768);
    viewer.show();


    return app.exec();
}
