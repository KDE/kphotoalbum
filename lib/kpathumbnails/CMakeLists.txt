# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

# The library is defined in the parent directory, this file only defines test cases.

include(ECMAddTests)
ecm_add_test(
    TestThumbnailCache.cpp
    TEST_NAME TestThumbnailCache
    LINK_LIBRARIES Qt5::Core Qt5::Test KPA::Thumbnails
    )
# vi:expandtab:tabstop=4 shiftwidth=4:
