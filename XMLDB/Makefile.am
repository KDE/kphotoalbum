noinst_LTLIBRARIES = \
    libxmldb.la

INCLUDES = \
	 -I$(srcdir)/.. $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

libxmldb_la_SOURCES = \
	Database.cpp XMLCategoryCollection.cpp XMLCategory.cpp XMLImageDateCollection.cpp NumberedBackup.cpp

METASOURCES = AUTO
