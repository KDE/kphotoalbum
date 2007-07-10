
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kphotoalbum_kexidb_sqlite3driver_PART_SRCS 
   sqliteconnection.cpp 
   sqlitedriver.cpp 
   sqlitecursor.cpp 
   sqlitekeywords.cpp 
   sqlitepreparedstatement.cpp 
   sqlitevacuum.cpp 
   sqliteadmin.cpp 
   sqlitealter.cpp )

kde4_add_plugin(kphotoalbum_kexidb_sqlite3driver ${kphotoalbum_kexidb_sqlite3driver_PART_SRCS})

target_link_libraries(kphotoalbum_kexidb_sqlite3driver  ${KDE4_KPARTS_LIBS} kexisql3 kphotoalbumkexidb kphotoalbumkexidbparser )

install(TARGETS kphotoalbum_kexidb_sqlite3driver DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kphotoalbum_kexidb_sqlite3driver.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#include ../../../Makefile.global
#
#kde_module_LTLIBRARIES = kphotoalbum_kexidb_sqlite3driver.la
#
#INCLUDES = -I$(srcdir)/../../../3rdparty/kexisql3/src -I$(srcdir)/../..  \
#	-I$(srcdir)/../../.. $(all_includes)
#
#kphotoalbum_kexidb_sqlite3driver_la_METASOURCES = AUTO
#
#kphotoalbum_kexidb_sqlite3driver_la_SOURCES = sqliteconnection.cpp sqlitedriver.cpp sqlitecursor.cpp \
#sqlitekeywords.cpp sqlitepreparedstatement.cpp sqlitevacuum.cpp sqliteadmin.cpp \
#sqlitealter.cpp
#
#kphotoalbum_kexidb_sqlite3driver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) \
# ../../../3rdparty/kexisql3/src/libkexisql3.la \
# ../../libkphotoalbumkexidb.la \
# ../../parser/libkphotoalbumkexidbparser.la
#
#kphotoalbum_kexidb_sqlite3driver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) 
#
#
#kde_services_DATA = kphotoalbum_kexidb_sqlite3driver.desktop
#
#
#KDE_CXXFLAGS += -DKEXIDB_SQLITE_DRIVER_EXPORT= -D__KEXIDB__= \
#	-include $(srcdir)/../../global.h
#
