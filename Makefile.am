SUBDIRS = images icons
INCLUDES = $(all_includes)

bin_PROGRAMS = kpalbum
kpalbum_SOURCES = main.cpp \
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
	mainviewui.ui \
	optionsdialog.ui \
	imageconfigui.ui \
	wellcomedialog.ui \
	editor.cpp \
	showbusycursor.cpp \
	displayarea.cpp \
	draw.cpp linedraw.cpp rectdraw.cpp circledraw.cpp drawlist.cpp

kpalbum_LDFLAGS = $(all_libraries) $(KDE_RPATH) -lktexteditor
kpalbum_LDADD = $(LIB_KIO) $(LIBJPEG)
METASOURCES = AUTO
