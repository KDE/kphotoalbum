noinst_LTLIBRARIES = libBrowser.la

INCLUDES =  -I$(srcdir)/.. $(all_includes)

if compile_with_exiv2
  EXIF_SOURCES = ExifFolder.cpp
endif

libBrowser_la_SOURCES = Browser.cpp ContentFolder.cpp Folder.cpp SearchFolder.cpp \
			BrowserItemFactory.cpp ImageFolder.cpp TypeFolder.cpp $(EXIF_SOURCES)



KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
