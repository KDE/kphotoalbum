#!/bin/bash
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0
##
# A quick and dirty script to extract all(?) hardcoded icon names and check if they are available on the current system.

BASEDIR="$(dirname "$0")/../.."


getIconList() {
	cd "$BASEDIR" || return
	# QIcon::fromTheme:
	git grep 'QIcon::fromTheme[(]' | sed 's/.*QIcon::fromTheme[(][^"]*"\([^"]*\)".*/\1/' | grep -v fromTheme

	# KIconLoader::global()->loadIcon:
	git grep 'KIconLoader::global()->loadIcon[(]' | sed 's/.*KIconLoader::global()->loadIcon[(][^"]*"\([^"]*\)".*/\1/'  | grep -v KIconLoader::

	# Via wrapper smallIcon:
	git grep 'smallIcon[(]' | sed 's/.*smallIcon[(][^"]*"\([^"]*\)".*/\1/' | grep -v smallIcon

	# From SettingsDialog:
	sed -n '/Data data/,/};/ { s/[^,]*,[^,]*, "\([^"]*\)".*/\1/ ; p }' Settings/SettingsDialog.cpp | grep '^[a-z]'

	# Special categories:
	grep "new DB::Category" DB/XML/FileReader.cpp | sed 's/.*new DB::Category([^,]*,[^"]*"\([^"]*\)".*/\1/' | grep -v DB::Category

}

getIconList | sort -u | {
	while read -r icon
	do
		kiconfinder5 "$icon" || echo "MISSING: $icon"
	done
}
