/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>

#include "ImageDetails.h"
#include "ImageProvider.h"
#include "ImageStore.h"
#include "MyImage.h"
#include "PositionObserver.h"
#include "RemoteImage.h"
#include "RemoteInterface.h"
#include "ScreenInfo.h"
#include "Settings.h"
#include "SlideShow.h"

using namespace RemoteControl;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("KDE");
    QCoreApplication::setOrganizationDomain("kde.org");
    QCoreApplication::setApplicationName("KPhotoAlbum");

    QQuickView viewer;
    PositionObserver::setView(&viewer);

    ScreenInfo::instance().setScreen(viewer.screen());
    QObject::connect(viewer.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    qmlRegisterType<RemoteImage>("KPhotoAlbum", 1, 0, "RemoteImage");
    qmlRegisterType<MyImage>("KPhotoAlbum", 1, 0, "MyImage");
    qmlRegisterUncreatableType<Types>("KPhotoAlbum", 1, 0, "Enums", "Don't create instances of this class");
    viewer.engine()->addImageProvider(QLatin1String("images"), &ImageProvider::instance());

    QQmlContext *rootContext = viewer.engine()->rootContext();
    rootContext->setContextProperty(QStringLiteral("_remoteInterface"), &RemoteInterface::instance());
    rootContext->setContextProperty(QStringLiteral("_settings"), &Settings::instance());
    rootContext->setContextProperty(QStringLiteral("_screenInfo"), &ScreenInfo::instance());
    rootContext->setContextProperty(QStringLiteral("_imageDetails"), &ImageDetails::instance());
    rootContext->setContextProperty(QStringLiteral("_slideShow"), new SlideShow);

    viewer.setSource(QStringLiteral("qrc:/qml/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);

    viewer.resize(1024, 768);
    viewer.show();

    // Create the store on the GUI thread
    (void)ImageStore::instance();

    return app.exec();
}
