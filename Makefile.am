if compile_LIBKIMDABA_COMPAT
  COMPAT_DIR = compat
  COMPAT_LIBS = compat/libkimdabacompat.la
  COMPAT_INCLUDE = -I$(srcdir)/compat
endif

SUBDIRS = images icons demo html_templates $(COMPAT_DIR)
INCLUDES = $(all_includes) $(COMPAT_INCLUDE)

bin_PROGRAMS = kimdaba
kimdaba_SOURCES = main.cpp \
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
	imagesearchinfo.cpp \
	editor.cpp \
	showbusycursor.cpp \
	displayarea.cpp \
	draw.cpp linedraw.cpp rectdraw.cpp circledraw.cpp drawlist.cpp \
	htmlexportdialog.cpp imagecounter.cpp iconviewtooltip.cpp \
	folder.cpp browser.cpp imagefolder.cpp typefolder.cpp contentfolder.cpp \
	imagedb.cpp showoptionaction.cpp \
	undoredolist.cpp optionsdialog.cpp infobox.cpp welcomedialog.cpp \
	viewhandler.cpp drawhandler.cpp searchfolder.cpp speeddisplay.cpp deletedialog.cpp \
	datefolder.cpp groupCounter.cpp membermap.cpp categoryimageconfig.cpp browseritemfactory.cpp \
	externalpopup.cpp

kimdaba_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimdaba_LDADD = $(LIB_KIO) $(COMPAT_LIBS) -lktexteditor -ljpeg

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_THREADS) $(KIMDABA_CXXFLAGS)

rcdir = $(kde_datadir)/kimdaba
rc_DATA = kimdabaui.rc default-setup

pkg_DATA = infobox.js
pkgdir = $(kde_datadir)/kimdaba/
EXTRA_DIST = $(pkg_DATA)

appsdatadir=$(kde_datadir)/kimdaba
appsdata_DATA=tips

xdg_apps_DATA = kimdaba.desktop

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	$(XGETTEXT) *.cpp *.h *.cc -o $(podir)/kimdaba.pot
	rm -f tips.cc 
