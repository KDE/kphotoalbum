#!/bin/bash
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

fail() {
	echo "$0: $@" >&2
	exit 1
}

year=$(date +%Y)
copyright="$(git config --get user.name) <$(git config --get user.email)>" || fail "git config failed to get user data!"
which reuse >/dev/null || fail "Reuse tool not installed! See https://reuse.software for information about the tool..."

if [[ "$#" -eq 0 ]]
then
	echo "Usage: $0 FILE.." >&2
	echo "" >&2
	echo "Call 'reuse addheader' on the given files to update the copyright information with your name, email and the current year." >&2
	echo "" >&2
	exit 1
fi

reuse addheader --year $year --copyright "$copyright" "$@"
