SUBDIRS = images icons demo
INCLUDES = $(all_includes)
AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

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
	kshell.cpp # needed for KDE 3.1.1 or older

kimdaba_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimdaba_LDADD = $(LIB_KIO) -lktexteditor -ljpeg

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_THREADS)

rcdir = $(kde_datadir)/kimdaba
rc_DATA = kimdabaui.rc default-setup

pkg_DATA = infobox.js
pkgdir = $(kde_datadir)/kimdaba/
EXTRA_DIST = $(pkg_DATA)

appsdatadir=$(kde_datadir)/kimdaba
appsdata_DATA=tips
