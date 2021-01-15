# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

add_library(kpathumbnails
    ThumbnailCache.cpp
    ThumbnailCache.h
    CacheFileInfo.cpp
    CacheFileInfo.h
    )
add_library(KPA::Thumbnails ALIAS kpathumbnails)
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpathumbnails PROPERTIES CXX_VISIBILITY_PRESET default)

target_include_directories(kpathumbnails
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/..
    )
target_link_libraries(kpathumbnails
    PUBLIC
    KPA::Base
    Qt5::Gui
    )

install(TARGETS kpathumbnails ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# vi:expandtab:tabstop=4 shiftwidth=4:
