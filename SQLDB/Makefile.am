noinst_LTLIBRARIES = \
    libsqldb.la

INCLUDES = \
	 -I$(srcdir)/.. $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) # -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

libsqldb_la_SOURCES = \
	Database.cpp Query.cpp SQLCategoryCollection.cpp SQLCategory.cpp SQLImageInfo.cpp QueryUtil.cpp SQLImageDateCollection.cpp

METASOURCES = AUTO
