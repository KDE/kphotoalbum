noinst_LTLIBRARIES = \
    libsqldb.la

INCLUDES = \
	 -I$(srcdir)/.. $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) # -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

libsqldb_la_SOURCES = \
	Database.cpp Query.cpp SQLCategoryCollection.cpp SQLFolderCategory.cpp SQLCategory.cpp SQLImageInfo.cpp QueryHelper.cpp KexiHelpers.cpp SQLImageDateCollection.cpp QueryErrors.cpp DatabaseHandler.cpp SQLMD5Map.cpp SQLImageInfoCollection.cpp Cursor.cpp

METASOURCES = AUTO
