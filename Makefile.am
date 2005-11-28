if compile_sqldb
  LIBKIMDABA_SQLDB=sqldb/libsqldb.la
  SQLDB=sqldb
else
  LIBKIMDABA_SQLDB=
  SQLDB=
endif

if compile_with_exiv2
  EXIF_SOURCES = exifinfo.cpp exiftreeview.cpp exifdialog.cpp 
endif

SUBDIRS = images icons demo themes survey xmldb $(SQLDB)
INCLUDES =  -I$(srcdir)/survey $(all_includes)

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
	groupCounter.cpp membermap.cpp categoryimageconfig.cpp browseritemfactory.cpp \
	externalpopup.cpp optionmatcher.cpp donate.cpp deletethumbnailsdialog.cpp \
	kdateedit.cpp viewersizeconfig.cpp export.cpp import.cpp importmatcher.cpp \
	mysplashscreen.cpp readinfodialog.cpp plugininterface.cpp myimageinfo.cpp myimagecollection.cpp \
	categoryimagecollection.cpp searchbar.cpp tokeneditor.cpp category.cpp categorycollection.cpp \
	datebar.cpp dateviewhandler.cpp imagedaterange.cpp \
	invaliddatefinder.cpp fileinfo.cpp imageinfolist.cpp datebarmousehandler.cpp \
	imagerequest.cpp thumbnailrequest.cpp mysurvey.cpp survey-count.cpp miniviewer.cpp \
	newimagefinder.cpp md5map.cpp imagedecoder.cpp rawimagedecoder.cpp parse.c \
	featuredialog.cpp $(EXIF_SOURCES)

kimdaba_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kimdaba_LDADD =  survey/libsurvey.la xmldb/libxmldb.la $(LIBKIMDABA_SQLDB) $(LIB_KIO) -ljpeg $(KIMDABA_KIPI_LIBS) $(KIMDABA_EXIV2_LIBS)

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII $(KIMDABA_KIPI_CXXFLAGS) $(KIMDABA_EXIV2_CXXFLAGS)

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

kde_conf_DATA = kimdabarc
