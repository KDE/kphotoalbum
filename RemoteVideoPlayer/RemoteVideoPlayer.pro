QT+= widgets network multimedia multimediawidgets
CONFIG += c++11
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp \
    Executor.cpp \
    MainWindow.cpp \
    PathMapper.cpp \
    PathMappingDialog.cpp

HEADERS += \
    Executor.h \
    MainWindow.h \
    PathMapper.h \
    PathMappingDialog.h

FORMS += \
    MainWindow.ui \
    PathMappingDialog.ui
