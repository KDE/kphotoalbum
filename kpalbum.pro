SOURCES	+= main.cpp \
	thumbnailview.cpp \
	thumbnail.cpp \
	imagemanager.cpp \
	imageloader.cpp \
	options.cpp \
	imageinfo.cpp \
	viewer.cpp \
	listselect.cpp \
	mainview.cpp \
	imageconfig.cpp \
	imageclient.cpp \
	util.cpp \
	imagepreview.cpp \
	imagedate.cpp \
	imagesearchinfo.cpp

HEADERS	+= thumbnailview.h \
	thumbnail.h \
	imagemanager.h \
	imageloader.h \
	options.h \
	imageinfo.h \
	viewer.h \
	listselect.h \
	mainview.h \
	imageconfig.h \
	imageclient.h \
	util.h \
	imagepreview.h \
	imagedate.h \
	imagesearchinfo.h

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

FORMS	= mainviewui.ui \
	optionsdialog.ui \
	imageconfigui.ui \
	wellcomedialog.ui

IMAGES	= images/splash.png \
	images/multiconfig.jpg \
	images/search.jpg

TEMPLATE	=app
CONFIG	+= thread
INCLUDEPATH	+= .
LIBS	+= -ljpeg
LANGUAGE	= C++
