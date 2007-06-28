
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libUtilities.la
#
#INCLUDES = -I$(srcdir)/.. $(all_includes)
#
#libUtilities_la_SOURCES = ShowBusyCursor.cpp Util.cpp
#
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII
#
#METASOURCES = AUTO
