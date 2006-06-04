noinst_LTLIBRARIES = \
    libThumbnailView.la

INCLUDES = \
	 -I$(srcdir)/.. $(all_includes)

libThumbnailView_la_SOURCES = \
         ThumbnailRequest.cpp  ThumbnailToolTip.cpp  ThumbnailWidget.cpp \
         GridResizeInteraction.cpp SelectionInteraction.cpp MouseTrackingInteraction.cpp \
	 ThumbnailBuilder.cpp 

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
