# SPDX-FileCopyrightText: 2009 Burkhard Lück <lueck@hube-lueck.de>
# SPDX-FileCopyrightText: 2016, 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2020 Luigi Toscano <luigi.toscano@tiscali.it>
# SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(KF6 REQUIRED DocTools)
kdoctools_create_handbook(index.docbook INSTALL_DESTINATION "${KDE_INSTALL_DOCBUNDLEDIR}/en" SUBDIR kphotoalbum)

# vi:expandtab:tabstop=4 shiftwidth=4:
