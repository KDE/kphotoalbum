#!/bin/bash
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0
##
# A quick and dirty script to extract all(?) hardcoded icon names and check if they are available on the current system.


getIconList() {
	# QIcon::fromTheme:
	git grep 'QIcon::fromTheme[(]' | sed 's/.*QIcon::fromTheme[(][^"]*"\([^"]*\)".*/\1/' | grep -v fromTheme

	# Via wrapper smallIcon:
	git grep 'smallIcon[(]' | sed 's/.*smallIcon[(][^"]*"\([^"]*\)".*/\1/' | grep -v smallIcon

	# From SettingsDialog:
	sed -n '/Data data/,/};/ { s/[^,]*,[^,]*, "\([^"]*\)".*/\1/ ; p }' Settings/SettingsDialog.cpp | grep '^[a-z]'

	# Special categories:
	grep "new DB::Category" DB/XML/FileReader.cpp | sed 's/.*new DB::Category([^,]*,[^"]*"\([^"]*\)".*/\1/' | grep -v DB::Category

}

getIconList | sort -u | {
	declare -a icons
   echo "Icon list:"
	while read -r icon
	do
		echo "$icon"
		icons+=("$icon")
	done
	echo "Checking icons:"
	for icon in "${icons[@]}"
	do
		kiconfinder5 "$icon" || echo "MISSING: $icon"
	done
}
