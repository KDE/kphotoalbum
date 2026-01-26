#
# SPDX-License-Identifier: BSD-3-Clause

# This file only defines test cases.

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
# vi:expandtab:tabstop=4 shiftwidth=4:
