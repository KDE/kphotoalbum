SOURCES	+= main.cpp \
	thumbnailview.cpp \
	thumbnail.cpp \
	imagemanager.cpp \
	imageloader.cpp \
	options.cpp \
	imageinfo.cpp \
	metainfo.cpp \
	viewer.cpp \
	listselect.cpp \
    mainview.cpp \
    imageconfig.cpp \
    imageclient.cpp

HEADERS	+= thumbnailview.h \
	thumbnail.h \
	imagemanager.h \
	imageloader.h \
	options.h \
	imageinfo.h \
	metainfo.h \
	viewer.h \
	listselect.h \
    mainview.h \
    imageconfig.h \
    imageclient.h

FORMS	= mainviewui.ui \
	optionsdialog.ui \
	imageconfigui.ui

unix {
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  UI_DIR = .ui
}
!unix {
  MOC_DIR = _moc
  OBJECTS_DIR = _obj
  UI_DIR = _ui
}

TEMPLATE	=app
INCLUDEPATH	+= .
LANGUAGE	= C++
LIBS += -ljpeg
