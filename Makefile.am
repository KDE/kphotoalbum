if compile_sqldb
  LIBKIMDABA_SQLDB=SQLDB/libsqldb.la
  SQLDB=SQLDB
endif

if compile_with_kipi
  KIMDABA_KIPI_LIBS=-lkipi
endif

if compile_with_exiv2
  EXIF_SOURCES = exiffolder.cpp
  EXIF_DIR = Exif
  KIMDABA_EXIV2_LIBS = Exif/libexif.la -lexiv2
endif

SUBDIRS = images icons demo themes Survey XMLDB DateBar $(SQLDB) $(EXIF_DIR) ThumbnailView
INCLUDES =  -I$(srcdir)/Survey $(all_includes)

bin_PROGRAMS = kimdaba
kimdaba_SOURCES = \
	main.cpp imagemanager.cpp imageloader.cpp \
	options.cpp imageinfo.cpp viewer.cpp listselect.cpp mainview.cpp imageconfig.cpp \
	imageclient.cpp util.cpp imagepreview.cpp imagedate.cpp imagesearchinfo.cpp \
	editor.cpp showbusycursor.cpp displayarea.cpp \
	draw.cpp linedraw.cpp rectdraw.cpp circledraw.cpp drawlist.cpp \
	htmlexportdialog.cpp imagecounter.cpp \
	folder.cpp browser.cpp imagefolder.cpp typefolder.cpp contentfolder.cpp \
	imagedb.cpp showoptionaction.cpp \
	optionsdialog.cpp infobox.cpp welcomedialog.cpp \
	viewhandler.cpp drawhandler.cpp searchfolder.cpp speeddisplay.cpp deletedialog.cpp \
	groupCounter.cpp membermap.cpp categoryimageconfig.cpp browseritemfactory.cpp \
	externalpopup.cpp optionmatcher.cpp donate.cpp deletethumbnailsdialog.cpp \
	kdateedit.cpp viewersizeconfig.cpp export.cpp import.cpp importmatcher.cpp \
	mysplashscreen.cpp plugininterface.cpp myimageinfo.cpp myimagecollection.cpp \
	categoryimagecollection.cpp searchbar.cpp tokeneditor.cpp category.cpp categorycollection.cpp \
	imagedaterange.cpp invaliddatefinder.cpp fileinfo.cpp imageinfolist.cpp \
	imagerequest.cpp miniviewer.cpp \
	newimagefinder.cpp md5map.cpp imagedecoder.cpp rawimagedecoder.cpp parse.c \
	featuredialog.cpp $(EXIF_SOURCES)

kimdaba_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimdaba_LDADD =  Survey/libsurvey.la XMLDB/libxmldb.la DateBar/libdatebar.la ThumbnailView/libThumbnailView.la \
                 $(LIBKIMDABA_SQLDB) $(LIB_KIO) -ljpeg $(KIMDABA_KIPI_LIBS) \
	         $(KIMDABA_EXIV2_LIBS)

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

rcdir = $(kde_datadir)/kimdaba
rc_DATA = kimdabaui.rc default-setup exif-sqlite.db

appsdatadir=$(kde_datadir)/kimdaba
appsdata_DATA=tips

xdg_apps_DATA = kimdaba.desktop kimdaba-import.desktop

mimetypeapplicationdatadir = $(kde_mimedir)/application
mimetypeapplicationdata_DATA = x-vnd.kde.kimdaba-import.desktop

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	$(XGETTEXT) *.cpp *.h *.cc -o $(podir)/kimdaba.pot
	rm -f tips.cc 

kde_conf_DATA = kimdabarc
