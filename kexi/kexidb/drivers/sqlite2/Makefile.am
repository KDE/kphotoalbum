include ../../../Makefile.global

kde_module_LTLIBRARIES = kphotoalbum_kexidb_sqlite2driver.la

INCLUDES = -I$(kphotoalbum_top_srcdir)/kexi \
	-I$(kphotoalbum_top_srcdir)/kexi/kexidb \
	-I$(kphotoalbum_top_srcdir)/kexi/3rdparty/kexisql/src \
	-I$(srcdir)/../sqlite \
	-I$(srcdir)/../sqlite/moc \
	-I../sqlite \
	$(all_includes)

kphotoalbum_kexidb_sqlite2driver_la_METASOURCES = AUTO

kphotoalbum_kexidb_sqlite2driver_la_SOURCES = sqliteconnection.cpp sqlitedriver.cpp sqlitecursor.cpp \
	sqlitepreparedstatement.cpp

kphotoalbum_kexidb_sqlite2driver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) \
 $(kphotoalbum_top_builddir)/kexi/3rdparty/kexisql/src/libkexisql2.la \
 $(kphotoalbum_top_builddir)/kexi/kexidb/libkphotoalbumkexidb.la \
 $(kphotoalbum_top_builddir)/kexi/kexidb/parser/libkphotoalbumkexidbparser.la

kphotoalbum_kexidb_sqlite2driver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) 

# we need the .mocs to be built
sqliteconnection.lo: ../sqlite/sqliteconnection.moc
sqlitedriver.lo: ../sqlite/sqlitedriver.moc

kde_services_DATA = kphotoalbum_kexidb_sqlite2driver.desktop


KDE_CXXFLAGS += -DKEXIDB_SQLITE_DRIVER_EXPORT= -D__KEXIDB__= \
	-DSQLITE2= -include $(kphotoalbum_top_srcdir)/kexi/kexidb/global.h

