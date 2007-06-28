
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kphotoalbumkexiutils_LIB_SRCS 
   identifier.cpp 
   validator.cpp 
   utils.cpp 
   debuggui.cpp 
   styleproxy.cpp 
   longlongvalidator.cpp )

kde4_automoc(${kphotoalbumkexiutils_LIB_SRCS})

kde4_add_library(kphotoalbumkexiutils SHARED ${kphotoalbumkexiutils_LIB_SRCS})

target_link_libraries(kphotoalbumkexiutils  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

set_target_properties(kphotoalbumkexiutils PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kphotoalbumkexiutils DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES  DESTINATION ${INCLUDE_INSTALL_DIR}/kexiutils )



#original Makefile.am contents follow:

#include ../Makefile.global
#
#lib_LTLIBRARIES = libkphotoalbumkexiutils.la
#libkphotoalbumkexiutils_la_SOURCES = identifier.cpp validator.cpp utils.cpp debuggui.cpp \
#  styleproxy.cpp longlongvalidator.cpp
#
#libkexiutilsincludedir=$(includedir)/kexiutils
#
#noinst_HEADERS = tristate.h identifier.h validator.h utils.h kexiutils_export.h \
#	styleproxy.h longlongvalidator.h utils_p.h
#
#libkphotoalbumkexiutils_la_LDFLAGS = \
#  $(KDE_RPATH) $(all_libraries) \
#  $(VER_INFO) -Wnounresolved -no-undefined
#
#libkphotoalbumkexiutils_la_LIBADD = $(LIB_QT) $(LIB_KDEUI) $(LIB_KIO) 
#
#INCLUDES = -I$(srcdir)/.. $(all_includes)
#
#METASOURCES = AUTO
