option( KPA_INSTALL_DEVSCRIPTS "Install shell scripts useful to developers." OFF)
if( KPA_INSTALL_DEVSCRIPTS )
	install( PROGRAMS dev/kpa-mktestdb.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
endif()

install( PROGRAMS open-raw.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( PROGRAMS open-raw.pl DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
install( PROGRAMS kpa-backup.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
