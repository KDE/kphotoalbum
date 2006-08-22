include ../Makefile.global

lib_LTLIBRARIES = libkphotoalbumkexidb.la

INCLUDES = -I$(srcdir)/.. $(all_includes)

SUBDIRS = . parser drivers

libkphotoalbumkexidb_la_METASOURCES = AUTO

libkphotoalbumkexidb_la_SOURCES = drivermanager.cpp driver.cpp driver_p.cpp connection.cpp \
 keywords.cpp object.cpp field.cpp utils.cpp expression.cpp \
 connectiondata.cpp fieldlist.cpp tableschema.cpp cursor.cpp transaction.cpp  \
 indexschema.cpp queryschema.cpp schemadata.cpp global.cpp relationship.cpp \
 roweditbuffer.cpp msghandler.cpp \
 dbobjectnamevalidator.cpp preparedstatement.cpp \
 dbproperties.cpp

noinst_HEADERS = drivermanager_p.h utils_p.h

kexidbincludedir=$(includedir)/kexidb
noinst_HEADERS += connection.h connectiondata.h cursor.h \
  driver.h drivermanager.h \
  error.h expression.h field.h fieldlist.h global.h object.h \
  schemadata.h tableschema.h queryschema.h indexschema.h \
  relationship.h parser/parser.h transaction.h preparedstatement.h \
  kexidb_export.h

libkphotoalbumkexidb_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KIO) \
	 ../kexiutils/libkphotoalbumkexiutils.la
libkphotoalbumkexidb_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kde_servicetypes_DATA = kphotoalbum_kexidb_driver.desktop


KDE_CXXFLAGS += -D__KEXIDB__= -include $(srcdir)/global.h

