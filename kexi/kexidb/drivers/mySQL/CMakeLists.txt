
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kphotoalbum_kexidb_mysqldriver_PART_SRCS 
   mysqldriver.cpp 
   mysqlconnection.cpp 
   mysqlconnection_p.cpp 
   mysqlcursor.cpp 
   mysqlkeywords.cpp 
   mysqlpreparedstatement.cpp )

kde4_add_plugin(kphotoalbum_kexidb_mysqldriver ${kphotoalbum_kexidb_mysqldriver_PART_SRCS})

target_link_libraries(kphotoalbum_kexidb_mysqldriver  ${KDE4_KPARTS_LIBS} mysqlclient kphotoalbumkexidb )

install(TARGETS kphotoalbum_kexidb_mysqldriver DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  kphotoalbum_kexidb_mysqldriver.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#include ../../../Makefile.global
#
#kde_module_LTLIBRARIES = kphotoalbum_kexidb_mysqldriver.la
#
#INCLUDES = -I$(MYSQL_INC) -I$(srcdir)/../../.. \
#  -I$(srcdir)/../..  \
#  -I$(srcdir)/../../.. $(all_includes)
#
#kphotoalbum_kexidb_mysqldriver_la_METASOURCES = AUTO
#
#kphotoalbum_kexidb_mysqldriver_la_SOURCES = \
#  mysqldriver.cpp \
#  mysqlconnection.cpp \
#  mysqlconnection_p.cpp \
#  mysqlcursor.cpp \
#  mysqlkeywords.cpp \
#  mysqlpreparedstatement.cpp
#
#kphotoalbum_kexidb_mysqldriver_la_LIBADD = $(LIB_KPARTS) \
#  $(LIB_QT) \
#  $(MYSQL_LIBS) \
#  -lmysqlclient \
#  ../../libkphotoalbumkexidb.la
#
#kphotoalbum_kexidb_mysqldriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) -no-undefined
#
#
#kde_services_DATA = kphotoalbum_kexidb_mysqldriver.desktop
#
#
#KDE_CXXFLAGS += -DKEXIDB_MYSQL_DRIVER_EXPORT= -D__KEXIDB__= \
#	-include $(srcdir)/../../global.h
#
