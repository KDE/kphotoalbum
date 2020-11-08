/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>

#include "ImageDetails.h"
#include "ImageStore.h"
#include "MyImage.h"
#include "PositionObserver.h"
#include "RemoteImage.h"
#include "RemoteInterface.h"
#include "ScreenInfo.h"
#include "Settings.h"

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

    QQmlContext *rootContext = viewer.engine()->rootContext();
    rootContext->setContextProperty(QStringLiteral("_remoteInterface"), &RemoteInterface::instance());
    rootContext->setContextProperty(QStringLiteral("_settings"), &Settings::instance());
    rootContext->setContextProperty(QStringLiteral("_screenInfo"), &ScreenInfo::instance());
    rootContext->setContextProperty(QStringLiteral("_imageDetails"), &ImageDetails::instance());

    viewer.setSource(QStringLiteral("qrc:/qml/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);

    viewer.resize(1024, 768);
    viewer.show();

    // Create the store on the GUI thread
    (void)ImageStore::instance();

    return app.exec();
}
