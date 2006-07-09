if compile_sqldb
  LIBKPHOTOALBUM_SQLDB=SQLDB/libsqldb.la
  SQLDB=SQLDB
endif

if compile_with_kipi
  KPHOTOALBUM_KIPI_LIBS=-lkipi
endif

if compile_with_exiv2
  EXIF_DIR = Exif
  KPHOTOALBUM_EXIV2_LIBS = Exif/libexif.la -lexiv2
endif

SUBDIRS = images icons demo themes Survey XMLDB DateBar $(SQLDB) $(EXIF_DIR) ThumbnailView Plugins Viewer Browser ImportExport \
          AnnotationDialog MainWindow Settings Utilities ImageManager DB

INCLUDES =  -I$(srcdir)/Survey $(all_includes)

bin_PROGRAMS = kphotoalbum
kphotoalbum_SOURCES = main.cpp
kphotoalbum_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kphotoalbum_LDADD =  MainWindow/libMainWindow.la Survey/libsurvey.la ImportExport/libImportExport.la \
		     XMLDB/libxmldb.la DateBar/libdatebar.la Browser/libBrowser.la \
		     Plugins/libPlugins.la Viewer/libViewer.la ThumbnailView/libThumbnailView.la \
		     AnnotationDialog/libAnnotationDialog.la Settings/libSettings.la \
		     Utilities/libUtilities.la ImageManager/libImageManager.la DB/libDB.la \
                     $(LIBKPHOTOALBUM_SQLDB) $(LIB_KIO) -ljpeg $(KPHOTOALBUM_KIPI_LIBS) \
	             $(KPHOTOALBUM_EXIV2_LIBS) -lkmediaplayer 

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

rcdir = $(kde_datadir)/kphotoalbum
rc_DATA = kphotoalbumui.rc default-setup default-layout.xml exif-sqlite.db

appsdatadir=$(kde_datadir)/kphotoalbum
appsdata_DATA=tips

xdg_apps_DATA = kphotoalbum.desktop kphotoalbum-import.desktop

mimetypeapplicationdatadir = $(kde_mimedir)/application
mimetypeapplicationdata_DATA = x-vnd.kde.kphotoalbum-import.desktop

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	$(XGETTEXT) *.cpp *.h *.cc -o $(podir)/kphotoalbum.pot
	rm -f tips.cc 

kde_conf_DATA = kphotoalbumrc
