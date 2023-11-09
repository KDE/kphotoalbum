# SPDX-FileCopyrightText: 2021 - 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
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

find_package(KF5KDcraw)
set_package_properties(KF5KDcraw
    PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Enables RAW image support"
    )
set(HAVE_KDCRAW ${KF5KDcraw_FOUND})

configure_file(kpabase/config-kpa-kdcraw.h.in "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-kdcraw.h")
configure_file(kpabase/config-kpa-plugins.h.in "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-plugins.h")
configure_file(kpabase/config-kpa-marble.h.in "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-marble.h")

add_library(kpabase
    "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-kdcraw.h"
    "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-plugins.h"
    "${CMAKE_CURRENT_BINARY_DIR}/kpabase/config-kpa-marble.h"
    kpabase/enums.cpp
    kpabase/enums.h
    kpabase/CrashSentinel.cpp
    kpabase/CrashSentinel.h
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
    kpabase/FileExtensions.cpp
    kpabase/FileExtensions.h
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
if(KF5KDcraw_FOUND)
    target_link_libraries(kpabase PRIVATE KF5::KDcraw)
endif()

install(TARGETS kpabase ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# unit tests:
add_subdirectory(kpabase)

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

# unit tests:
add_subdirectory(kpathumbnails)

################################################################################
### KPA::Exif
add_library(kpaexif
    kpaexif/Database.cpp
    kpaexif/Database.h
    kpaexif/SearchInfo.cpp
    kpaexif/SearchInfo.h
    kpaexif/Info.cpp
    kpaexif/Info.h
    kpaexif/DatabaseElement.cpp
    kpaexif/DatabaseElement.h
    )
add_library(KPA::Exif ALIAS kpaexif)
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpaexif PROPERTIES CXX_VISIBILITY_PRESET default)

### 2018-12-30 jzarl
# When Exiv2 0.26 can be deprecated, FindExiv2.cmake should be removed
# and only find_package(exiv2) should be used
find_package(exiv2 CONFIG QUIET)
if(exiv2_FOUND)
    # search again with REQUIRED, so that the feature summary correctly shows exiv as required dependency
    find_package(exiv2 CONFIG REQUIRED)
    # exiv2 0.28.1 moved exiv2lib into the Exiv2 namespace:
    if(TARGET Exiv2::exiv2lib)
        set(EXIV2_LIBRARIES Exiv2::exiv2lib)
    else()
        set(EXIV2_LIBRARIES exiv2lib)
    endif()
else()
    find_package(Exiv2 REQUIRED)
endif()

target_link_libraries(kpaexif
    PRIVATE
    Qt5::Sql
    PUBLIC
    ${EXIV2_LIBRARIES} # TODO(jzarl): make this private if possible
    KPA::Base
    Qt5::Core
    )

install(TARGETS kpaexif ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# unit tests:
add_subdirectory(kpaexif)

# vi:expandtab:tabstop=4 shiftwidth=4:
