include ../../../Makefile.global

kde_module_LTLIBRARIES = kphotoalbum_kexidb_pqxxsqldriver.la

INCLUDES = -I$(srcdir)/../../.. $(all_includes) -I$(PG_INCDIR) -I$(PQXX_INCDIR)

kphotoalbum_kexidb_pqxxsqldriver_la_METASOURCES = AUTO

kphotoalbum_kexidb_pqxxsqldriver_la_SOURCES = pqxxdriver.cpp pqxxcursor.cpp pqxxconnection.cpp \
	pqxxkeywords.cpp pqxxconnection_p.cpp pqxxpreparedstatement.cpp

kphotoalbum_kexidb_pqxxsqldriver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) -lpqxx ../../libkphotoalbumkexidb.la

kphotoalbum_kexidb_pqxxsqldriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) \
	-L$(PQXX_LIBDIR) -L$(PG_LIBDIR) $(VER_INFO) -no-undefined

kde_services_DATA = kphotoalbum_kexidb_pqxxsqldriver.desktop

noinst_HEADERS =  pqxxconnection.h pqxxconnection_p.h

KDE_CXXFLAGS += -DKEXIDB_PGSQL_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(srcdir)/../../global.h
