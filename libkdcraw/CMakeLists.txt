project(libkdcraw)

# Depencies detection require to compile dcraw.c
# Detect Little CMS library properlly with CMake script from 'cmake/modules'
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(LCMS REQUIRED)

# Detect libJPEG
find_package(JPEG REQUIRED)

set(DCRAW_LIB_VERSION "2.0.0")
set(DCRAW_LIB_SOVERSION "2")


subdirs(icons)
subdirs(libkdcraw)
subdirs(dcraw)

if(NOT WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libkdcraw.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkdcraw.pc)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkdcraw.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)
