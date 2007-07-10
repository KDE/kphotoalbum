
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kphotoalbum_kexidb_odbcdriver_PART_SRCS odbcdriver.cpp odbcconnection.cpp )

kde4_add_plugin(kphotoalbum_kexidb_odbcdriver ${kphotoalbum_kexidb_odbcdriver_PART_SRCS})

target_link_libraries(kphotoalbum_kexidb_odbcdriver  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} odbc kphotoalbumkexidb )

install(TARGETS kphotoalbum_kexidb_odbcdriver DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kphotoalbum_kexidb_odbcdriver.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#include ../../../Makefile.global
#
#kde_module_LTLIBRARIES = kphotoalbum_kexidb_odbcdriver.la
#
#INCLUDES = -I$(srcdir)/../..  -I$(srcdir)/../../.. $(all_includes)
#
#kphotoalbum_kexidb_odbcdriver_la_METASOURCES = AUTO
#
#kphotoalbum_kexidb_odbcdriver_la_SOURCES = odbcdriver.cpp odbcconnection.cpp 
#
#kphotoalbum_kexidb_odbcdriver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) -lodbc ../../libkphotoalbumkexidb.la
#
#kphotoalbum_kexidb_odbcdriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) -no-undefined
#
#
#kde_services_DATA = kphotoalbum_kexidb_odbcdriver.desktop
#
#
#KDE_CXXFLAGS += -DKEXIDB_ODBC_DRIVER_EXPORT= -D__KEXIDB__= \
#	-include $(srcdir)/../../global.h
#
