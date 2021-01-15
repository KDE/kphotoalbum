# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

add_custom_target(
    UpdateVersion ALL
    COMMAND "${CMAKE_COMMAND}"
    "-DBASE_DIR=${kphotoalbum_SOURCE_DIR}" -DPROJECT_NAME=KPA -DPROJECT_VERSION="${kphotoalbum_VERSION}"
    -DOUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" -P "${kphotoalbum_SOURCE_DIR}/cmake/modules/UpdateVersion.cmake"
    COMMENT "Updating version header."
    BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/version.h"
    )

configure_file(config-kpa-kdcraw.h.in "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-kdcraw.h")
configure_file(config-kpa-plugins.h.in "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-plugins.h")
configure_file(config-kpa-marble.h.in "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-marble.h")

add_library(kpabase
    "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-kdcraw.h"
    "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-plugins.h"
    "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-marble.h"
    enums.cpp
    enums.h
    FileName.cpp
    FileName.h
    FileNameList.cpp
    FileNameList.h
    Logging.cpp
    Logging.h
    UIDelegate.cpp
    UIDelegate.h
    SettingsData.cpp
    SettingsData.h
    FileNameUtil.cpp
    FileNameUtil.h
    StringSet.cpp
    StringSet.h
    version.h
    )
add_library(KPA::Base ALIAS kpabase)
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpabase PROPERTIES CXX_VISIBILITY_PRESET default)

# see CMP0071
set_property(SOURCE version.h PROPERTY SKIP_AUTOMOC ON)

target_include_directories(kpabase
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/..
    )
target_link_libraries(kpabase
    PUBLIC
    KF5::ConfigCore
    KF5::I18n
    Qt5::Core
    )

install(TARGETS kpabase ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# vi:expandtab:tabstop=4 shiftwidth=4:
