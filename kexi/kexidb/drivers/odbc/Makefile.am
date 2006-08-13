include ../../../Makefile.global

noinst_LTLIBRARIES = kexidb_odbcdriver.la

INCLUDES = -I$(srcdir)/../..  -I$(kphotoalbum_top_srcdir)/kexi $(all_includes)

kexidb_odbcdriver_la_METASOURCES = AUTO

kexidb_odbcdriver_la_SOURCES = odbcdriver.cpp odbcconnection.cpp 

kexidb_odbcdriver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) -lodbc $(kphotoalbum_top_builddir)/kexi/kexidb/libkexidb.la

kexidb_odbcdriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) -no-undefined


kde_services_DATA = kphotoalbum_kexidb_odbcdriver.desktop


KDE_CXXFLAGS += -DKEXIDB_ODBC_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(kphotoalbum_top_srcdir)/kexi/kexidb/global.h

