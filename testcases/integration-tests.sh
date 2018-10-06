#!/bin/bash
# Copyright 2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

export LC_ALL=C
myname=`basename "$0"`
mydir=`dirname "$0"`

DEPS="kphotoalbum convert exiv2 kdialog"
# basic kphotoalbumrc options to make testing less annoying (i.e. prevent some pop-ups):
BASE_RC="\n[Thumbnails]\ndisplayCategories=true\n[Notification Messages]\nimage_config_typein_show_help=false\n[TipOfDay]\nRunOnStart=false"
TEMPDIR=
KEEP_TEMPDIR=0
declare -A _checks _context

result_ok=0
result_failed=1
result_err_crash=2
result_err_setup=3

declare -A LOG_LEVELS
LOG_LEVELS[debug]=0
LOG_LEVELS[info]=1
LOG_LEVELS[notice]=2
LOG_LEVELS[warning]=3
LOG_LEVELS[err]=4
# default log level:
LOG_LEVEL=2

### functions

cleanup()
{
	if [[ "$KEEP_TEMPDIR" == 1 ]]
	then
		log info "NOT removing temporary directory '$TEMPDIR'."
	else
		if [[ -d "$TEMPDIR" ]]
		then
			log debug "Removing '$TEMPDIR'..."
			rm -rf "$TEMPDIR"
		fi
	fi
}

log()
{
	lvl="$1"
	shift
	if [ "${LOG_LEVELS[$lvl]}" -ge "$LOG_LEVEL" ]
	then
		echo "$myname[$lvl]: $*" >&2
	fi
}

print_help()
{
	echo "Usage: $myname --check ] [PARAMETERS...] [--all|CHECKS...]" >&2
	echo "       $myname --help" >&2
	echo "       $myname --list" >&2
	echo "       $myname --print" >&2
	echo "" >&2
	echo "List or run integration tests for KPhotoAlbum." >&2
	echo "This script allows guided integration tests that present the user with concrete things to check." >&2
	echo "" >&2
	echo "Modes:" >&2
	echo "-c|--check                                  Run the specified checks." >&2
	echo "-l|--list                                   List available checks." >&2
	echo "-p|--print                                  Print available checks with description." >&2
	echo "" >&2
	echo "Parameters:" >&2
	echo "--all                                       Run all tests (only valid for --check)." >&2
	echo "--keep-tempdir                              Do not remove temporary files." >&2
	echo "--log-level debug|info|notice|warning|err   Set log level (default: notice)." >&2
	echo "--tempdir DIR                               Use DIR for temporary files (implies --keep-tempdir)." >&2
	echo "" >&2
}

setup_check()
# setup_check DIR
# sets up a demo db in DIR/db
{
	local check_dir="$1"
	if ! mkdir "$check_dir" "$check_dir/db" "$check_dir/QtProject"
	then
		log err "Could not create check directories for prefix '$check_dir'!"
		return 1
	fi

	# set logging rules
	cat > "$check_dir/QtProject/qtlogging.ini" <<EOF
[Rules]
*=false
kphotoalbum.*=true
EOF

	# copy demo database
	if ! cp -r "$mydir/../demo/"* "$check_dir/db"
	then
		log err "Could not copy demo database to '$check_dir/db'!"
		return 1
	fi
}

do_list()
{
	for check in "${!_checks[@]}"
	do
		echo "$check"
	done | sort
}

do_print()
{
	for check in "${!_checks[@]}"
	do
		echo "$check -- ${_checks[$check]}"
	done | sort
}

do_checks()
{
	let num_total=0
	let num_ok=0
	let num_failed=0
	let num_err_crash=0
	let num_err_setup=0

	for name
	do
		let num_total++
		do_check "$name"
		case "$?" in
			$result_ok)
				log info "$name: OK"
				let num_ok++
				;;
			$result_failed)
				log info "$name: FAILED"
				let num_failed++
				;;
			$result_err_crash)
				log info "$name: ERROR (crash)"
				let num_err_crash++
				;;
			$result_err_setup)
				log info "$name: ERROR (setup failed)"
				let num_err_setup++
				;;
			*)
				log err "Internal error: invalid return code while running '$name'!"
				exit 1
		esac
	done

	log notice "Summary: $num_ok of $num_total OK, $num_failed failed, $(( num_err_crash + num_err_setup)) errors."

	# return ok if no test failed:
	test "$num_total" -eq "$num_ok"
}

do_check()
{
	local check_name="$1"
	local check_desc="${_checks[$check_name]}"
	if [ -n "$check_desc" ]
	then
		log info "Running check $check_name ($check_desc)..."
		"$check_name"
	else
		log err "No check named '$check_name'!"
		exit 1
	fi
}

generic_check()
{
	local check_name="$1"
	local check_dir="$TEMPDIR/$check_name"
	setup_check "$check_dir" || return $result_err_setup
	kdialog --msgbox "<h1>$check_name</h1>${_context[$check_name]}"
	export XDG_CONFIG_HOME="$check_dir"
	prepare_$check_name "$check_dir"
	call_$check_name "$check_dir" > "$check_dir/log" 2>&1 || return $result_err_crash
	if kdialog --yesno "<h1>$check_name &mdash; Did KPhotoAlbum pass the test?</h1><p>As a reminder what you should check:</p><hr/><div style='text-size=small'>${_context[$check_name]}</div>"
	then
		return $result_ok
	else
		return $result_failed
	fi
}


### MAIN

for dep in $DEPS
do
	if ! which "$dep" >/dev/null 2>&1
	then
		log err "Could not find required dependency '$dep'!"
		exit 2
	fi
done

version=`kphotoalbum --version 2>&1`
log info "Using $version (`which kphotoalbum`)..."

TEMP=`getopt -o clhp --long "all,check,help,keep-tempdir,list,log-level:,print,tempdir:" -n "$myname" -- "$@"`
if [ $? != 0 ] ; then log err "Terminating..." ; exit 1 ; fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

MODE=check
while true ; do
	case "$1" in
		-h|--help) print_help ; exit ;;
		-l|--list) MODE="list" ; shift ;;
		-c|--check) MODE="check" ; shift ;;
		-p|--print) MODE="print" ; shift ;;
		--keep-tempdir) KEEP_TEMPDIR=1 ; shift ;;
		--tempdir) TEMPDIR="$2" ; KEEP_TEMPDIR=1 ; shift 2 ;;
		--all) RUN_ALL=1 ; shift ;;
		--log-level) LOG_LEVEL="${LOG_LEVELS[$2]}" ; shift 2 ;;
		--) shift ; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done

if [ -z "$TEMPDIR" ]
then
	TEMPDIR=`mktemp -d --tmpdir kphotoalbum-tests-XXXXXX`
fi

trap cleanup EXIT

# read test files
for f in "$mydir/integration-tests/"*.sh
do
	. "$f"
done

case $MODE in
	list|print)
		do_$MODE
		;;
	check)
		if [[ "$RUN_ALL" == 1 ]]
		then
			eval set -- "${!_checks[@]}"
		fi
		do_checks "$@"
		;;
esac
