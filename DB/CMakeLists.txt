# SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
#
# SPDX-License-Identifier: BSD-3-Clause

# This file only defines test cases.

# add_subdirectory(search)

# For Utilities/FastDateTime.h:
include_directories(..)

include(ECMAddTests)

ecm_add_test(
   TestImageDate.cpp
   ImageDate.cpp
   ../Utilities/FastDateTime.cpp
   TEST_NAME TestImageDate
   LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Base
   )

# ecm_add_test(
#    search/TestImageSearchInfo.cpp
#    search/ImageSearchInfo.cpp
#    TEST_NAME TestImageSearchInfo
#    LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Base
#    )

# vi:expandtab:tabstop=4 shiftwidth=4:
