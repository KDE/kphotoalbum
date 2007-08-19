prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libkdcraw
Description: KDE interface library for dcraw command line program to decode RAW picture files 
URL: http://www.kipi-plugins.org
Requires:
Version: 0.2.0
Libs: -L${LIB_INSTALL_DIR} -lkdcraw
Cflags: -I${INCLUDE_INSTALL_DIR}
