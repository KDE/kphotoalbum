#!/bin/bash

# SPDX-FileCopyrightText: 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

DB="$1"

if [ ! -r "$DB" ]
then
	echo "Usage: $0 /path/to/index.xml" >&2
	echo "Shows the file locations for all category icons in an index.xml file." >&2
	echo >&2
	exit 1
fi

list_category_icons()
# list_category_icons INDEX.XML
# list all icon names for category icons
{
	local DB="$1"
	sed -E -n '/\W*<Category/ s/.*icon="([^"]+)".*/\1/p' "$DB"
}

files_for_icons()
# filter to match icons to their icon files
{
	while read icon
	do
		kiconfinder "$icon" || echo "$icon not found" >&2
	done
}

list_category_icons "$DB" | files_for_icons
