include_directories(${KDE4_INCLUDES} ${QT4_INCLUDES})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

set(kdcraw_LIB_SRCS dcrawbinary.cpp kdcraw.cpp dcrawsettingswidget.cpp)

kde4_add_library(kdcraw SHARED ${kdcraw_LIB_SRCS})

target_link_libraries(kdcraw ${KDE4_KDECORE_LIBS} 
                             ${KDE4_KDEUI_LIBS} 
			     ${QT_QT3SUPPORT_LIBRARY}
                     )

set_target_properties(kdcraw PROPERTIES VERSION ${DCRAW_LIB_VERSION} SOVERSION ${DCRAW_LIB_SOVERSION})

install(TARGETS kdcraw DESTINATION ${LIB_INSTALL_DIR})

install(FILES rawdecodingsettings.h kdcraw.h dcrawbinary.h dcrawsettingswidget.h dcrawinfocontainer.h rawfiles.h libkdcraw_export.h ${CMAKE_CURRENT_BINARY_DIR}/version.h DESTINATION ${INCLUDE_INSTALL_DIR}/libkdcraw)

