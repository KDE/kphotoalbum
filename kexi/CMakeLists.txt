
add_subdirectory( 3rdparty )
add_subdirectory( kexiutils )
add_subdirectory( kexidb )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = 3rdparty kexiutils kexidb
#
#DOXYGEN_EXCLUDE = 3rdparty kexidb/parser/sqlparser.h kexidb/drivers/odbc
#
#
#
