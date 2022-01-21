QT += qml quick network widgets
CONFIG += c++11
TARGET=KPhotoAlbum

SOURCES += main.cpp \
    ImageProvider.cpp \
    RemoteInterface.cpp \
    RemoteImage.cpp \
    Client.cpp \
    CategoryModel.cpp \
    MyImage.cpp \
    Settings.cpp \
    ImageStore.cpp \
    ScreenInfo.cpp \
    Action.cpp \
    History.cpp \
    ThumbnailModel.cpp \
    ImageDetails.cpp \
    DiscoveryModel.cpp \
    PositionObserver.cpp \
    ../RemoteControl/RemoteConnection.cpp \
    ../RemoteControl/RemoteCommand.cpp \
    ../RemoteControl/SearchInfo.cpp \

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
    RemoteImage.h \
    Client.h \
    CategoryModel.h \
    MyImage.h \
    Settings.h \
    ImageStore.h \
    ScreenInfo.h \
    Action.h \
    History.h \
    ThumbnailModel.h \
    ImageDetails.h \
    DiscoveryModel.h \
    PositionObserver.h \
    ../RemoteControl/RemoteConnection.h \
    ../RemoteControl/RemoteCommand.h \
    ../RemoteControl/SearchInfo.h \
    ../RemoteControl/Types.h \
    ../RemoteControl/Serializer.h

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
