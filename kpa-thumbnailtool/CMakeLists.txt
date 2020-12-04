# SPDX-FileCopyrightText: 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

add_executable(kpa-thumbnailtool
    main.cpp
    Logging.cpp
    Logging.h
    ThumbnailCacheConverter.cpp
    ThumbnailCacheConverter.h
    )
target_link_libraries(kpa-thumbnailtool PRIVATE
    KPA::Thumbnails
    KF5::CoreAddons
    )

install(TARGETS kpa-thumbnailtool ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

include(ECMAddTests)
ecm_add_test(
    TestThumbnailCacheConverter.h
    TestThumbnailCacheConverter.cpp
    ThumbnailCacheConverter.cpp
    ThumbnailCacheConverter.h
    TEST_NAME ThumbnailCacheConverter
    LINK_LIBRARIES Qt5::Core Qt5::Test KF5::I18n
    )


# vi:expandtab:tabstop=4 shiftwidth=4:
