include ../../../Makefile.global

kde_module_LTLIBRARIES = kphotoalbum_kexidb_odbcdriver.la

INCLUDES = -I$(srcdir)/../..  -I$(srcdir)/../../.. $(all_includes)

kphotoalbum_kexidb_odbcdriver_la_METASOURCES = AUTO

kphotoalbum_kexidb_odbcdriver_la_SOURCES = odbcdriver.cpp odbcconnection.cpp 

kphotoalbum_kexidb_odbcdriver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) -lodbc ../../libkexidb.la

kphotoalbum_kexidb_odbcdriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) -no-undefined


kde_services_DATA = kphotoalbum_kexidb_odbcdriver.desktop


KDE_CXXFLAGS += -DKEXIDB_ODBC_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(srcdir)/../../global.h

