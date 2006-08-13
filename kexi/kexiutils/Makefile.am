include ../Makefile.global

noinst_LTLIBRARIES = libkexiutils.la
libkexiutils_la_SOURCES = identifier.cpp validator.cpp utils.cpp

libkexiutilsincludedir=$(includedir)/kexiutils

noinst_HEADERS = tristate.h identifier.h validator.h utils.h kexiutils_export.h utils_p.h

libkexiutils_la_LDFLAGS = \
  $(KDE_RPATH) $(all_libraries) \
  $(VER_INFO) -Wnounresolved -no-undefined

libkexiutils_la_LIBADD = $(LIB_QT) $(LIB_KDEUI) $(LIB_KIO) 

INCLUDES = -I$(kphotoalbum_top_srcdir)/kexi $(all_includes)

METASOURCES = AUTO
