# Copyright 2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Redistribution and use is allowed according to the terms of the BSD 3-clause license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


###############################
# Explanation:
# Marble changed the signature of the MarbleWidget::regionSelected signal in v18.03.80 from:
#   regionSelected(const QList<double>&)
# to:
#   regionSelected(const Marble::GeoDataLatLonBox &)
# [see https://cgit.kde.org/marble.git/commit/?id=ec1f7f554e9f6ca248b4a3b01dbf08507870687e]
#
# These feature checks are necessary, because MARBLE_VERSION was not changed in
# the release and we are left without a way to just check the version of
# marble.
#
# Note: as you can see, the checks exploit the fact that signals are actually just methods,
# and that one can "call" a signal. A more elaborate check that connects the signal to a
# custom class fails because check_cxx_source_compiles does not play well with the MOC.

include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_LIBRARIES Marble Qt5::Widgets)

# Marble 17.12.3
check_cxx_source_compiles(
    "#include <QList>\n\
    #include <marble/MarbleWidget.h>\n\
    int main() {\n\
    Marble::MarbleWidget mw;\n\
    QList<double> arg;\n\
    mw.regionSelected(arg);\n\
    return 0;\n\
    }"
    MARBLE_HAS_regionSelected_OLD
    )

# Marble 18.03.80
check_cxx_source_compiles(
    "#include <marble/GeoDataLatLonBox.h>\n\
    #include <marble/MarbleWidget.h>\n\
    int main() {\n\
    Marble::MarbleWidget mw;\n\
    Marble::GeoDataLatLonBox arg;\n\
    mw.regionSelected(arg);\n\
    return 0;\n\
    }"
    MARBLE_HAS_regionSelected_NEW
    )

if(MARBLE_HAS_regionSelected_OLD AND MARBLE_HAS_regionSelected_NEW)
    message(AUTHOR_WARNING "Both old and new style for Marble::MarbleWidget::regionSelected are valid!")
endif()

if(NOT MARBLE_HAS_regionSelected_OLD AND NOT MARBLE_HAS_regionSelected_NEW)
    message(SEND_ERROR "No suitable signature for Marble::MarbleWidget::regionSelected! Please file a bug report against kphotoalbum!")
endif()
# vi:expandtab:tabstop=4 shiftwidth=4:
