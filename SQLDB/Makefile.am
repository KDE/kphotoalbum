noinst_LTLIBRARIES = \
    libsqldb.la

INCLUDES = \
         -I$(srcdir)/../kexi \
	 -I$(srcdir)/.. $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) # -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

libsqldb_la_SOURCES = \
	Database.cpp \
	SQLCategoryCollection.cpp \
	SQLFolderCategory.cpp \
	SQLSpecialCategory.cpp \
	SQLCategory.cpp \
	SQLImageInfo.cpp \
	QueryHelper.cpp \
	SQLImageDateCollection.cpp \
	DatabaseHandler.cpp \
	SQLMD5Map.cpp \
	SQLImageInfoCollection.cpp \
	Cursor.cpp \
	ConfigFileHandler.cpp \
	SQLSettingsWidget.cpp \
	DatabaseAddress.cpp \
	SQLMemberMap.cpp \
	QueryResult.cpp \
	SQLTagCategory.cpp \
	SQLNormalCategory.cpp \
	SQLTokensCategory.cpp

METASOURCES = AUTO
