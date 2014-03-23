QT += qml quick network widgets
CONFIG += c++11

SOURCES += main.cpp \
    RemoteInterface.cpp \
    RemoteConnection.cpp \
    RemoteImage.cpp \
    RemoteCommand.cpp \
    Client.cpp

OTHER_FILES += \
    qml/main.qml \
    qml/CategoryPage.qml

RESOURCES += \
    resources.qrc

HEADERS += \
    RemoteInterface.h \
    RemoteConnection.h \
    RemoteImage.h \
    RemoteCommand.h \
    Client.h
