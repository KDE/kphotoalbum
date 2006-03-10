noinst_LTLIBRARIES = libImportExport.la

INCLUDES =  -I$(srcdir)/.. $(all_includes)

libImportExport_la_SOURCES = Export.cpp Import.cpp ImportMatcher.cpp XMLHandler.cpp MiniViewer.cpp

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
