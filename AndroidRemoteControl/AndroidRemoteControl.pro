QT += qml quick network widgets
CONFIG += c++11
TARGET=KPhotoAlbum

SOURCES += main.cpp \
    ImageProvider.cpp \
    RemoteInterface.cpp \
    RemoteConnection.cpp \
    RemoteImage.cpp \
    RemoteCommand.cpp \
    Client.cpp \
    CategoryModel.cpp \
    MyImage.cpp \
    SearchInfo.cpp \
    Settings.cpp \
    ImageStore.cpp \
    ScreenInfo.cpp \
    Action.cpp \
    History.cpp \
    ThumbnailModel.cpp \
    ImageDetails.cpp \
    DiscoveryModel.cpp \
    PositionObserver.cpp

OTHER_FILES += \
    qml/main.qml \
    qml/Icon.qml \
    qml/OverviewPage.qml \
    qml/ThumbnailsPage.qml \
    qml/ImageViewer.qml \
    qml/ScrollBar.qml \
    android/AndroidManifest.xml \
    qml/ImageDetails.qml \
    qml/CategoryListView.qml \
    qml/PositionObserver.qml \
    qml/Keyboard.qml \
    qml/Cell.qml

RESOURCES += \
    resources.qrc

HEADERS += \
    ImageProvider.h \
    RemoteInterface.h \
    RemoteConnection.h \
    RemoteImage.h \
    RemoteCommand.h \
    Client.h \
    CategoryModel.h \
    MyImage.h \
    SearchInfo.h \
    Settings.h \
    ImageStore.h \
    ScreenInfo.h \
    Types.h \
    Action.h \
    History.h \
    ThumbnailModel.h \
    ImageDetails.h \
    DiscoveryModel.h \
    PositionObserver.h \
    Serializer.h

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
