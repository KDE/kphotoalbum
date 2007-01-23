include ../Makefile.global

lib_LTLIBRARIES = libkphotoalbumkexiutils.la
libkphotoalbumkexiutils_la_SOURCES = identifier.cpp validator.cpp utils.cpp debuggui.cpp \
  styleproxy.cpp longlongvalidator.cpp

libkexiutilsincludedir=$(includedir)/kexiutils

noinst_HEADERS = tristate.h identifier.h validator.h utils.h kexiutils_export.h \
	styleproxy.h longlongvalidator.h utils_p.h

libkphotoalbumkexiutils_la_LDFLAGS = \
  $(KDE_RPATH) $(all_libraries) \
  $(VER_INFO) -Wnounresolved -no-undefined

libkphotoalbumkexiutils_la_LIBADD = $(LIB_QT) $(LIB_KDEUI) $(LIB_KIO) 

INCLUDES = -I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO
