noinst_LTLIBRARIES = libViewer.la

INCLUDES =  -I$(srcdir)/.. $(all_includes)

libViewer_la_SOURCES = Draw.cpp CircleDraw.cpp RectDraw.cpp LineDraw.cpp DrawList.cpp Viewer.cpp DrawHandler.cpp DisplayArea.cpp \
                       ViewHandler.cpp SpeedDisplay.cpp ShowOptionAction.cpp InfoBox.cpp

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
