# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

################################################################################
### KPA::Base
add_custom_target(
    UpdateVersion ALL
    COMMAND "${CMAKE_COMMAND}"
    "-DBASE_DIR=${kphotoalbum_SOURCE_DIR}" -DPROJECT_NAME=KPA -DPROJECT_VERSION="${kphotoalbum_VERSION}"
    "-DOUTPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/kpabase"
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" -P "${kphotoalbum_SOURCE_DIR}/cmake/modules/UpdateVersion.cmake"
    COMMENT "Updating version header."
    BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/kpabase/version.h"
    )

configure_file(kpabase/config-kpa-kdcraw.h.in "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-kdcraw.h")
configure_file(kpabase/config-kpa-plugins.h.in "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-plugins.h")
configure_file(kpabase/config-kpa-marble.h.in "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-marble.h")

add_library(kpabase
    "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-kdcraw.h"
    "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-plugins.h"
    "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-marble.h"
    kpabase/enums.cpp
    kpabase/enums.h
    kpabase/FileName.cpp
    kpabase/FileName.h
    kpabase/FileNameList.cpp
    kpabase/FileNameList.h
    kpabase/Logging.cpp
    kpabase/Logging.h
    kpabase/UIDelegate.cpp
    kpabase/UIDelegate.h
    kpabase/SettingsData.cpp
    kpabase/SettingsData.h
    kpabase/FileNameUtil.cpp
    kpabase/FileNameUtil.h
    kpabase/StringSet.cpp
    kpabase/StringSet.h
    kpabase/version.h
    )
add_library(KPA::Base ALIAS kpabase)
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpabase PROPERTIES CXX_VISIBILITY_PRESET default)

# see CMP0071
set_property(SOURCE kpabase/version.h PROPERTY SKIP_AUTOMOC ON)

target_link_libraries(kpabase
    PUBLIC
    KF5::ConfigCore
    KF5::I18n
    Qt5::Core
    )

install(TARGETS kpabase ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

################################################################################
### KPA::Thumbnails
add_library(kpathumbnails
    kpathumbnails/ThumbnailCache.cpp
    kpathumbnails/ThumbnailCache.h
    kpathumbnails/CacheFileInfo.cpp
    kpathumbnails/CacheFileInfo.h
    )
add_library(KPA::Thumbnails ALIAS kpathumbnails)
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpathumbnails PROPERTIES CXX_VISIBILITY_PRESET default)

target_link_libraries(kpathumbnails
    PUBLIC
    KPA::Base
    Qt5::Gui
    )

install(TARGETS kpathumbnails ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# vi:expandtab:tabstop=4 shiftwidth=4:
