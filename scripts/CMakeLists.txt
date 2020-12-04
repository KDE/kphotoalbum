# SPDX-FileCopyrightText: 2014, 2016-2018, 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2020 Miika Turkia <miika.turkia@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

install( PROGRAMS org.kde.kphotoalbum.open-raw.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install( PROGRAMS open-raw.pl kpa-backup.sh DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" )

# vi:expandtab:tabstop=4 shiftwidth=4:
