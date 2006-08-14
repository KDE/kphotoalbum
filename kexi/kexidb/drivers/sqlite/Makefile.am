include ../../../Makefile.global

kde_module_LTLIBRARIES = kphotoalbum_kexidb_sqlite3driver.la

INCLUDES = -I$(kphotoalbum_top_srcdir)/kexi/3rdparty/kexisql3/src -I$(srcdir)/../..  \
	-I$(kphotoalbum_top_srcdir)/kexi $(all_includes)

kphotoalbum_kexidb_sqlite3driver_la_METASOURCES = AUTO

kphotoalbum_kexidb_sqlite3driver_la_SOURCES = sqliteconnection.cpp sqlitedriver.cpp sqlitecursor.cpp \
sqlitekeywords.cpp sqlitepreparedstatement.cpp

kphotoalbum_kexidb_sqlite3driver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) \
 $(kphotoalbum_top_builddir)/kexi/3rdparty/kexisql3/src/libkexisql3.la \
 $(kphotoalbum_top_builddir)/kexi/kexidb/libkphotoalbumkexidb.la \
 $(kphotoalbum_top_builddir)/kexi/kexidb/parser/libkphotoalbumkexidbparser.la

kphotoalbum_kexidb_sqlite3driver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) 


kde_services_DATA = kphotoalbum_kexidb_sqlite3driver.desktop


KDE_CXXFLAGS += -DKEXIDB_SQLITE_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(kphotoalbum_top_srcdir)/kexi/kexidb/global.h

