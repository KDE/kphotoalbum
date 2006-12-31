noinst_LTLIBRARIES = libDB.la

INCLUDES = -I$(srcdir)/.. $(all_includes)

libDB_la_SOURCES = ImageInfo.cpp Category.cpp CategoryCollection.cpp ImageDate.cpp MD5Map.cpp MemberMap.cpp \
		     ImageInfoList.cpp ImageDB.cpp FileInfo.cpp NewImageFinder.cpp GroupCounter.cpp CategoryMatcher.cpp \
		     ImageSearchInfo.cpp CategoryItem.cpp


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
