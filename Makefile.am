SUBDIRS = images icons
INCLUDES = $(all_includes)

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
	optionsdialog.ui \
	imageconfigui.ui \
	wellcomedialog.ui \
	editor.cpp \
	showbusycursor.cpp \
	displayarea.cpp \
	draw.cpp linedraw.cpp rectdraw.cpp circledraw.cpp drawlist.cpp \
	infopage.cpp htmlexportdialog.cpp imagecounter.cpp iconviewtooltip.cpp

kimdaba_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimdaba_LDADD = $(LIB_KIO) -lktexteditor

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_THREADS)

rcdir = $(kde_datadir)/kimdaba
rc_DATA = kimdabaui.rc

pkg_DATA = infobox.js
pkgdir = $(kde_datadir)/kimdaba/
EXTRA_DIST = $(pkg_DATA)
