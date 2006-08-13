include ../../../Makefile.global

noinst_LTLIBRARIES = kexidb_mysqldriver.la

INCLUDES = -I$(MYSQL_INC) -I$(srcdir)/../../.. \
  -I$(srcdir)/../..  \
  -I$(kphotoalbum_top_srcdir)/kexi $(all_includes)

kexidb_mysqldriver_la_METASOURCES = AUTO

kexidb_mysqldriver_la_SOURCES = \
  mysqldriver.cpp \
  mysqlconnection.cpp \
  mysqlconnection_p.cpp \
  mysqlcursor.cpp \
  mysqlkeywords.cpp \
  mysqlpreparedstatement.cpp

kexidb_mysqldriver_la_LIBADD = $(LIB_KPARTS) \
  $(LIB_QT) \
  $(MYSQL_LIBS) \
  -lmysqlclient \
  ../../libkexidb.la

kexidb_mysqldriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) -no-undefined


kde_services_DATA = kphotoalbum_kexidb_mysqldriver.desktop


KDE_CXXFLAGS += -DKEXIDB_MYSQL_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(kphotoalbum_top_srcdir)/kexi/kexidb/global.h

