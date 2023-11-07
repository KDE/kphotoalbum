#!/bin/bash
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0
##
# A quick and dirty script to extract all(?) hardcoded icon names and check if they are available on the current system.


getIconList() {
	echo "QIcon::fromTheme():" >&2
	git grep QIcon::fromTheme | sed 's/.*QIcon::fromTheme([^"]*"\([^"]*\)".*/\1/' | grep -v fromTheme | sort -u

	echo >&2
	echo "Via wrapper smallIcon():" >&2
	git grep smallIcon | sed 's/.*smallIcon([^"]*"\([^"]*\)".*/\1/' | grep -v smallIcon | sort -u

	echo >&2
	echo "From SettingsDialog:" >&2
	sed -n '/Data data/,/};/ { s/[^,]*,[^,]*, "\([^"]*\)".*/\1/ ; p }' Settings/SettingsDialog.cpp | grep '^[a-z]' | sort -u

	echo >&2
	echo "Special categories:" >&2
	grep "new DB::Category" DB/XML/FileReader.cpp | sed 's/.*new DB::Category([^,]*,[^"]*"\([^"]*\)".*/\1/' | grep -v DB::Category | sort -u

}

getIconList | sort -u | {
	declare -a icons
	while read -r icon
	do
		echo "$icon" >&2
		icons+=("$icon")
	done
	echo >&2
	echo "Checking icons:" >&2
	for icon in "${icons[@]}"
	do
		kiconfinder5 "$icon" || echo "MISSING: $icon" >&2
	done
}
