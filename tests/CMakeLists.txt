# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
# SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
#
# SPDX-License-Identifier: BSD-3-Clause

# This file only defines test cases.

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/DB
    ${CMAKE_SOURCE_DIR}/Utilities
    ${CMAKE_SOURCE_DIR}/lib/kpabase
    ${CMAKE_SOURCE_DIR}/lib/kpathumbnails
    ${CMAKE_SOURCE_DIR}/lib/kpaexif
    ${CMAKE_SOURCE_DIR}/kpa-thumbnailtool
    )

ecm_add_test(
    TestFileName.cpp
    TEST_NAME TestFileName
    LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Base
    )
ecm_add_test(
    TestFileNameUtil.cpp
    TEST_NAME TestFileNameUtil
    LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Base
    )

ecm_add_test(
    TestThumbnailCache.cpp
    TEST_NAME TestThumbnailCache
    LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Thumbnails
    )

ecm_add_test(
    TestExifDatabase.cpp
    TEST_NAME TestExifDatabase
    LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Exif
    )

ecm_add_test(
   TestImageDate.cpp
   ../DB/ImageDate.cpp
   ../Utilities/FastDateTime.cpp
   TEST_NAME TestImageDate
   LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Base
   )

ecm_add_test(
   TestTimespan.cpp
   ../Utilities/FastDateTime.cpp
   ../Utilities/Timespan.cpp
   ../DB/ImageDate.cpp
   TEST_NAME TestTimespan
   LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Base
   )

ecm_add_test(
    TestThumbnailCacheConverter.h
    TestThumbnailCacheConverter.cpp
    ../kpa-thumbnailtool/ThumbnailCacheConverter.cpp
    TEST_NAME ThumbnailCacheConverter
    LINK_LIBRARIES Qt6::Core Qt6::Test KF6::I18n
    )

# vi:expandtab:tabstop=4 shiftwidth=4:
