option( KPA_INSTALL_DEVSCRIPTS "Install shell scripts useful to developers." OFF)
if( KPA_INSTALL_DEVSCRIPTS )
	set ( DEVSCRIPTS kpa-mktestdb.sh )
endif()

install( PROGRAMS org.kde.kphotoalbum.open-raw.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( PROGRAMS open-raw.pl kpa-backup.sh ${DEVSCRIPTS} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

# vi:expandtab:tabstop=4 shiftwidth=4:
