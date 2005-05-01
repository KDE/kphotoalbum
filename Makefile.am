if compile_LIBKIMDABA_COMPAT
  COMPAT_DIR = compat
  COMPAT_LIBS = compat/libkimdabacompat.la
  COMPAT_INCLUDE = -I$(srcdir)/compat
endif

if HAVE_KIPI
  KIPI_LIBS = -lkipi
endif

SUBDIRS = images icons demo themes survey $(COMPAT_DIR)
INCLUDES = $(all_includes) $(COMPAT_INCLUDE) -I$(srcdir)/survey

bin_PROGRAMS = kimdaba
kimdaba_SOURCES = \
	main.cpp thumbnailview.cpp thumbnail.cpp imagemanager.cpp imageloader.cpp \
	options.cpp imageinfo.cpp viewer.cpp listselect.cpp mainview.cpp imageconfig.cpp \
	imageclient.cpp util.cpp imagepreview.cpp imagedate.cpp imagesearchinfo.cpp \
	editor.cpp showbusycursor.cpp displayarea.cpp \
	draw.cpp linedraw.cpp rectdraw.cpp circledraw.cpp drawlist.cpp \
	htmlexportdialog.cpp imagecounter.cpp iconviewtooltip.cpp \
	folder.cpp browser.cpp imagefolder.cpp typefolder.cpp contentfolder.cpp \
	imagedb.cpp showoptionaction.cpp \
	optionsdialog.cpp infobox.cpp welcomedialog.cpp \
	viewhandler.cpp drawhandler.cpp searchfolder.cpp speeddisplay.cpp deletedialog.cpp \
	datefolder.cpp groupCounter.cpp membermap.cpp categoryimageconfig.cpp browseritemfactory.cpp \
	externalpopup.cpp optionmatcher.cpp donate.cpp deletethumbnailsdialog.cpp thumbnailbuilder.cpp \
	kdateedit.cpp viewersizeconfig.cpp export.cpp import.cpp importmatcher.cpp \
	mysplashscreen.cpp readinfodialog.cpp plugininterface.cpp myimageinfo.cpp myimagecollection.cpp \
	categoryimagecollection.cpp searchbar.cpp tokeneditor.cpp category.cpp categorycollection.cpp \
    datebar.cpp dateviewhandler.cpp imagedaterangecollection.cpp imagedaterange.cpp \
	invaliddatefinder.cpp fileinfo.cpp imageinfolist.cpp datebarmousehandler.cpp \
	imagerequest.cpp thumbnailrequest.cpp mysurvey.cpp survey-count.cpp miniviewer.cpp

kimdaba_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimdaba_LDADD = $(LIB_KIO) $(COMPAT_LIBS) -ljpeg $(KIPI_LIBS) survey/libsurvey.la

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_THREADS) $(KIMDABA_CXXFLAGS) $(KIMDABA_KIPI_CXXFLAGS)

rcdir = $(kde_datadir)/kimdaba
rc_DATA = kimdabaui.rc default-setup

appsdatadir=$(kde_datadir)/kimdaba
appsdata_DATA=tips

xdg_apps_DATA = kimdaba.desktop kimdaba-import.desktop

mimetypeapplicationdatadir = $(kde_mimedir)/application
mimetypeapplicationdata_DATA = x-vnd.kde.kimdaba-import.desktop

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	$(XGETTEXT) *.cpp *.h *.cc -o $(podir)/kimdaba.pot
	rm -f tips.cc 
