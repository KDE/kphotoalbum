noinst_LTLIBRARIES = libImageManager.la

INCLUDES = -I$(srcdir)/.. $(all_includes)

libImageManager_la_SOURCES = ImageLoader.cpp Manager.cpp ImageRequest.cpp ImageClient.cpp \
			     ImageDecoder.cpp RawImageDecoder.cpp parse.c VideoManager.cpp \
			     RequestQueue.cpp


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
