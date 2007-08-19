#-- dcraw binary for KDE -------------------------------------------------

# libkdcraw use a dedicaced dcraw binary program to run. This is mandatory 
# since dcraw author have broken the command line options 
# compatibility with the 8.x serie.
# Also, dcraw is not available like a library.
# The dcraw.c source code is just a copy of official implementation, but 
# this one have been completly tested with libkdcraw core. 
# Note: dcraw project page is http://cybercom.net/~dcoffin

# Require by dcraw.c implementation.
KDE4_NO_ENABLE_FINAL(kdraw)


# This line is require to prevent broken compilation with -std=iso9899:1990 gcc option.
# In fact, dcraw.c use lcms.h witch use the "inline" keyword witch do not exist in pure C 
# implementation. The -Dinline option please compiler to see inline with this option.
# Note : using -std-c99 instead -std=iso9899:1990 is not possible because some compiler 
# do not support it.
# Thanks to Andras Mantia for this tip.
add_definitions("-Dinline= -w")

set(kdcraw_SRCS dcraw.c)
kde4_add_executable(kdcraw ${kdcraw_SRCS})
target_link_libraries(kdcraw ${JPEG_LIBRARY} ${LCMS_LIBRARIES})

# the target kdcraw binary programe name add the soname of library as suffixe.
set_target_properties(kdcraw PROPERTIES OUTPUT_NAME kdcraw-${DCRAW_LIB_VERSION})

# Note: we don't install kcraw man page. It will never used by users.
install(TARGETS kdcraw DESTINATION ${BIN_INSTALL_DIR})

