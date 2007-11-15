noinst_LTLIBRARIES = \
    libsqldb.la

INCLUDES = \
	 -I$(srcdir)/.. $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) # -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

libsqldb_la_SOURCES = \
	Database.cpp \
	KexiConnection.cpp \
	SQLCategoryCollection.cpp \
	SQLFolderCategory.cpp \
	SQLSpecialCategory.cpp \
	SQLCategory.cpp \
	SQLImageInfo.cpp \
	QueryHelper.cpp \
	SQLImageDateCollection.cpp \
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
	SQLTokensCategory.cpp \
	DriverManager.cpp \
	DatabaseManagers.cpp \
	DatabaseInitialization.cpp \
	Schema/DatabaseSchema.cpp \
	Schema/KPhotoAlbumSchema.cpp \
	Schema/CreateStatementGenerator.cpp \
	Schema/MySQLCSG.cpp \
	Schema/PostgreSQLCSG.cpp \
	Schema/SQLiteCSG.cpp

METASOURCES = AUTO
