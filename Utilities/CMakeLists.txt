# SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
#
# SPDX-License-Identifier: BSD-3-Clause

# This file only defines test cases.

# For DB/ImageDate.h:
include_directories(..)

include(ECMAddTests)

ecm_add_test(
   TestTimespan.cpp
   FastDateTime.cpp
   Timespan.cpp
   ../DB/ImageDate.cpp
   TEST_NAME TestTimespan
   LINK_LIBRARIES Qt6::Core Qt6::Test KPA::Base
   )

# vi:expandtab:tabstop=4 shiftwidth=4:
