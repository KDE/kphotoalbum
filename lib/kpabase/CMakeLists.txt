# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

add_library(kpabase
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
    )
add_library(KPA::Base ALIAS kpabase)
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpabase PROPERTIES CXX_VISIBILITY_PRESET default)

target_include_directories(kpabase
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/..
    )
target_link_libraries(kpabase
    PUBLIC
    KF5::ConfigCore
    KF5::I18n
    Qt5::Core
    )

# vi:expandtab:tabstop=4 shiftwidth=4:
