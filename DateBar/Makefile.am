noinst_LTLIBRARIES = \
    libdatebar.la

INCLUDES = \
	 -I$(srcdir)/.. $(all_includes)

libdatebar_la_SOURCES = DateBar.cpp ViewHandler.cpp MouseHandler.cpp

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
