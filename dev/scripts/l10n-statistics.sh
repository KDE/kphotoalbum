#!/bin/sh -e
# Print sorted statistics for all po files in the po directory.
# Don't forget the one source of truth for localization data:
# https://l10n.kde.org/stats/gui/trunk-kf5/po/kphotoalbum.po/

# SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-KDE-Accepted-LGPL

cd po
for lang in *
do
	test -d "$lang" || continue
	printf '%12s ' "$lang"
	LC_ALL=C msgfmt -o /dev/null --statistics "$lang"/*.po 2>&1
done | sort -r -g -k'1.13'
