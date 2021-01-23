# SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

# The library is defined in the parent directory, this file only defines test cases.

include(ECMAddTests)
ecm_add_test(
    TestFileName.cpp
    TEST_NAME TestFileName
    LINK_LIBRARIES Qt5::Core Qt5::Test KPA::Base
    )
ecm_add_test(
    TestFileNameUtil.cpp
    TEST_NAME TestFileNameUtil
    LINK_LIBRARIES Qt5::Core Qt5::Test KPA::Base
    )
# vi:expandtab:tabstop=4 shiftwidth=4:
