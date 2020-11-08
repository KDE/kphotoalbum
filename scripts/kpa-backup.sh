#!/bin/bash
# SPDX-FileCopyrightText: 2012-2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

QTPATHS=qtpaths
if ! $QTPATHS --types >/dev/null
then
	echo "QStandardPaths command line client ($QTPATHS) not usable!" >&2
	exit 1
fi

# default locations:
KPARC=`$QTPATHS --locate-file ConfigLocation kphotoalbumrc`
KPAUIRC=`$QTPATHS --locate-file GenericDataLocation kxmlgui5/kphotoalbum/kphotoalbumui.rc`
if [ -z "$KPAUIRC" ]
then
	echo "Info: using old location for ui.rc files..." >&2
	KPAUIRC=`$QTPATHS --locate-file GenericDataLocation kphotoalbum/kphotoalbumui.rc`
fi
BACKUP_LOCATION=~/kpa-backup
BACKUP_ID=latest
ACTION=
ADD_FILES_RELATIVE="exif-info.db layout.dat"
KEEP_NUM=5
TERSE=
NO_ACT=

SQLITE=sqlite3

###
# Helper functions:
###

get_config_value()
{
	sed -n 's/#.*// ; s/'$1'=\(.*\)/\1/p' "$KPARC"
}

resolve_link()
# Use readlink to resolve the given filename.
# If the file is no symlink, just return the filename.
{
	if [ -L "$1" ]
	then
		readlink "$1"
	else
		echo "$1"
	fi
}

print_help()
{
	echo "Usage: $0 -b|--backup OPTIONS..." >&2
	echo "       $0 -r|--restore OPTIONS..." >&2
	echo "       $0 -l|--list [--terse] OPTIONS..." >&2
	echo "       $0 -i|--info OPTIONS..." >&2
	echo "       $0 -p|--purge [--keep NUM] OPTIONS..." >&2
	echo "" >&2
	echo "Create or restore a backup of your essential KPhotoAlbum files." >&2
	echo "Note: your actual image-files are not backed up!" >&2
	echo "" >&2
	echo "Options:" >&2
	echo "-d|--directory BACKUP_LOCATION   Use the specified path as backup location" >&2
	echo "                                 [default: $BACKUP_LOCATION]" >&2
	echo "--id BACKUP_ID                   Use given backup instead of latest.">&2
	echo "-n|--no-act                      Do not take any action." >&2
	echo "" >&2
	echo "List options:" >&2
	echo "--terse                          Only show backup ids, no change information." >&2
	echo "" >&2
	echo "Purge options:" >&2
	echo "--keep NUM                       Keep the latest NUM backups." >&2
	echo "                                 [default: $KEEP_NUM]" >&2
	echo "" >&2
	echo "Actions:" >&2
	echo "-b|--backup" >&2
	echo "                                 Create a new backup." >&2
	echo "-r|--restore" >&2
	echo "                                 Restore the latest backup (or the one given by --id)." >&2
	echo "-l|--list" >&2
	echo "                                 List all backups, in the same format as --info." >&2
	echo "                                 If --terse is given: show a list of all backup ids." >&2
	echo "-i|--info" >&2
	echo "                                 Show which files in the latest backup (or the one specified by --id)" >&2
	echo "                                 have changed compared to the current state." >&2
	echo "-p|--purge" >&2
	echo "                                 Delete all but the latest $KEEP_NUM backups." >&2
	echo "" >&2
}

sqlite_diff()
# sqlite_diff SRC DST QUERYTEXT
# visual comparison for exif-info.db
# Either SRC or DST may be "-" to denote input from stdin.
# QUERYTEXT should be a descriptive query for the db, but even it does
# not have to include every aspect of the data.
# After the data-diff on the query, a diff on the whole file will determine
# the status of sqlite_diff.
{
	local src="$1"
	local dst="$2"
	local query="$3"
	# catch "exif_diff - -":
	if [ "$src" = "$dst" ]
	then
		echo "Identical files!" >&2
		return 0
	fi

	if ! "$SQLITE" -version >/dev/null
	then
		echo "Warning: sqlite not found." >&2
		diff -q -s "$1" "$2"
	else
		local tmp=`mktemp`
		local srctmp=`mktemp`
		local dsttmp=`mktemp`
		if [ "$src" = "-" ]
		then
			src="$tmp"
			cat > "$tmp"
		fi
		if [ "$dst" = "-" ]
		then
			dst="$tmp"
			cat > "$tmp"
		fi

		"$SQLITE" "$src" "$query" > "$srctmp"
		"$SQLITE" "$dst" "$query" > "$dsttmp"

		# display unified diff of query result:
		diff -u --label "$1" --label "$2" "$srctmp" "$dsttmp"
		# diff the actual file:
		diff -q --label "$1" --label "$2" "$src" "$dst"
		local retval=$?

		# cleanup
		rm "$tmp" "$srctmp" "$dsttmp"
		return $retval
	fi
}

exif_diff()
# exif_diff SRC DST
{
	sqlite_diff "$1" "$2" "select * from exif"
}

visual_diff()
# visual_diff [-q] SRC DST
# Do a diff of two files (or STDIN and a file).
# An appropriate diff format is used, based on the given file.
# If -q is given, no output is given.
# The exit value is just like from GNU diff.
{
	local quiet
	local src
	local dst
	local filename
	for param
	do
		case "$param" in
			-q) #quiet,question
				quiet=1
				;;
			*)
				if [ -z "$src" ]
				then
					src="$param"
				else if [ -z "$dst" ]
					then
						dst="$param"
					else
						echo "visual_diff: incorrect number of parameters!" >&2
						exit 1
					fi
				fi
				# we need a valid filename to determine which specialized diff we should use
				# usually, one parameter is "-", so we take the param that's a real filename
				[ -f "$param" ] && filename="$param"
				;;
		esac
	done

	if [ -n "$quiet" ]
	then
		# just do the quickest thing:
		diff -q "$src" "$dst" >/dev/null
	else
		filename=`basename "$filename"`
		case "$filename" in
			exif-info.db)
				exif_diff "$src" "$dst"
				;;
			index.xml)
				diff -u -F '^    <.*' "$src" "$dst"
				;;
			kphotoalbumui.rc)
				diff -u --ignore-space-change -F '^\ *<\(Menu\|ToolBar\).*' "$src" "$dst"
				;;
			kphotoalbumrc)
				diff -u -F '^\[.*\]' "$src" "$dst"
				;;
			*)
				# data; nothing to see:
				diff -q "$src" "$dst" >/dev/null
				;;
		esac
	fi
}

untar_if_changed()
# untar the given single-file-tarball if the destination file has changed
# use first parameter -p to print when the file has changed, but not untar it.
{
	local printonly=false
	[ -n "$NO_ACT" ] && printonly=true
	local quiet
	local tarfile
	for param
	do
		case "$param" in
			-p)
				printonly=true
				;;
			-q) #quiet
				quiet=-q
				;;
			*)
				tarfile="$param"
				;;
		esac
	done
	[ -f "$tarfile" ] || return 1
	local dstfile=`tar -Ptz -f "$tarfile"`
	if tar -PxzO -f "$tarfile" | visual_diff $quiet - "$dstfile"
	then
		echo "unchanged: $dstfile"
	else
		if $printonly
		then
			echo "  changed: $dstfile"
		else
			tar -Pwxvz -f "$tarfile"
		fi
	fi
}

do_backup()
{
	local INDEXFILE=
	local KPA_FOLDER=
	###
	# Query file locations from RC files & check parameter sanity:
	###

	if [ ! -r "$KPARC" ]
	then
		echo "RC-file ($KPARC) not readable!" >&2
		exit 1
	fi
	# KPA gets the image directory from the imageDBFile entry
	INDEXFILE=`get_config_value imageDBFile`
	if [ -z "$INDEXFILE" ]
	then
		echo "The RC-file ($KPARC) does not define an entry for index.xml!" >&2
		exit 1
	fi
	if [ ! -f "$INDEXFILE" ]
	then
		echo "KPhotoAlbum index file does not exist!" >&2
		exit 1
	fi
	KPA_FOLDER=`dirname "$INDEXFILE"`
	if [ ! -d "$KPA_FOLDER" ]
	then
		echo "KPhotoAlbum image directory ($KPA_FOLDER) does not exist!" >&2
		exit 1
	fi

	if [ ! -d "$BACKUP_LOCATION" ]
	then
		echo "Backup location ($BACKUP_LOCATION) is not a directory, creating it." >&2
		[ -z "$NO_ACT" ] && mkdir "$BACKUP_LOCATION" || exit 1
	fi
	BACKUP_LOCATION_WDATE="$BACKUP_LOCATION"/"`date +%Y%m%d-%H%M%S`"
	echo "Writing backup to $BACKUP_LOCATION_WDATE"
	[ -z "$NO_ACT" ] && mkdir "$BACKUP_LOCATION_WDATE"
	if [ -e "$BACKUP_LOCATION"/latest ]
	then
		[ -z "$NO_ACT" ] && rm "$BACKUP_LOCATION"/latest
	fi
	[ -z "$NO_ACT" ] && ln -s "$BACKUP_LOCATION_WDATE" "$BACKUP_LOCATION"/latest

	echo "Backing up essential files..."
	if [ -z "$NO_ACT" ]
	then
		for f in "$KPARC" "$INDEXFILE"
		do
			tar -Pcvz -f "$BACKUP_LOCATION_WDATE"/`basename "$f"`.tgz "$f"
		done
	fi
	echo "Backing up additional files..."
	if [ -z "$NO_ACT" ]
	then
		[ -f "$KPAUIRC" ] && tar -Pcvz -f "$BACKUP_LOCATION_WDATE"/`basename "$KPAUIRC"`.tgz "$KPAUIRC"
		for f in $ADD_FILES_RELATIVE
		do
			[ -f "$KPA_FOLDER/$f" ] && tar -Pcvz -f "$BACKUP_LOCATION_WDATE"/`basename "$f"`.tgz "$KPA_FOLDER/$f"
		done
	fi
}

do_restore()
{
	if [ ! -d "$BACKUP_LOCATION" ]
	then
		echo "Backup location ($BACKUP_LOCATION) is not a directory!" >&2
		exit 1
	fi
	echo "Restoring essential files..."
	for f in "$BACKUP_LOCATION/$BACKUP_ID"/*.tgz
	do
		# untar_if_changed honors NO_ACT:
		untar_if_changed "$f"
	done
}

show_info()
# show_info BACKUP_DIR [ANNOTATION]
# shows if a given backup location has changes to the current state
# if ANNOTATON is given, is is written next to the backup time
{
	backup_dir="$1"
	backup_name=`basename "$1"`
	annotation="$2"
	echo -n " -$backup_name" | sed 's/\(....\)\(..\)\(..\)-\(..\)\(..\)\(..\)/\0 [\1-\2-\3 \4:\5:\6]/'
	if [ -n "$annotation" ]
	then
		echo -n " $annotation"
	fi
	echo
	for f in "$backup_dir"/*.tgz
	do
		echo -n "  |-"
		untar_if_changed -p -q "$f"
	done
	echo
}

do_list()
{
	if [ ! -d "$BACKUP_LOCATION" ]
	then
		echo "Backup location ($BACKUP_LOCATION) is not a directory!" >&2
		exit 1
	fi
	local LATEST=`resolve_link "$BACKUP_LOCATION/latest"`
	LATEST=`basename "$LATEST"`
	local action=show_info
	[ -n "$TERSE" ] && action=basename
	echo "$BACKUP_LOCATION:"
	for d in "$BACKUP_LOCATION"/*
	do
		if [ -d "$d" ]
		then
			[ -L "$d" ] && continue
			if [ "`basename "$d"`" = "$LATEST" ]
			then
				$action "$d" "(*latest*)"
			else
				$action "$d"
			fi
		fi
	done
}

do_info()
{
	if [ ! -d "$BACKUP_LOCATION" ]
	then
		echo "Backup location ($BACKUP_LOCATION) is not a directory!" >&2
		exit 1
	fi
	local LATEST=`resolve_link "$BACKUP_LOCATION/$BACKUP_ID"`
	echo "$BACKUP_LOCATION:"
	show_info "$LATEST"
}

do_purge()
{
	if [ ! -d "$BACKUP_LOCATION" ]
	then
		echo "Backup location ($BACKUP_LOCATION) is not a directory!" >&2
		exit 1
	fi
	cd "$BACKUP_LOCATION"
	# list newest entries first, skip KEEP_NUM directories:
	for d in `ls -t1`
	do
		if [ -h "$d" -o ! -d "$d" ]
		then # skip "latest"
			echo "Skipping non-directory $d"
			continue
		fi
		if [ "$KEEP_NUM" -gt 0 ]
		then
			echo "Skipping $d..."
			let KEEP_NUM--
			continue
		fi
		echo "Purging backup $d..."
		[ -z "$NO_ACT" ] && rm -rf "$d"
	done
}

###
# Parse commandline:
###

TEMP=`getopt -o hbrlipnd: --long help,backup,restore,list,info,purge,no-act,directory:,keep:,id:,terse \
     -n 'kpa-backup' -- "$@"`

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

while true ; do
	case "$1" in
		-h|--help) print_help ; exit ;;
		-b|--backup) ACTION=do_backup ; shift ;;
		-r|--restore) ACTION=do_restore ; shift ;;
		-l|--list) ACTION=do_list ; shift ;;
		-i|--info) ACTION=do_info ; shift ;;
		-p|--purge) ACTION=do_purge ; shift ;;
		-n|--no-act) NO_ACT=1 ; shift ;;
		-d|--directory) BACKUP_LOCATION="$2" ; shift 2 ;;
		--keep) KEEP_NUM="$2" ; shift 2 ;;
		--id) BACKUP_ID="$2" ; shift 2 ;;
		--terse) TERSE=1 ; shift ;;
		--) shift ; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done

if [ "$#" -gt 0 ]
then
	echo "Unknown extra parameters: $@" >&2
	exit 1
fi

###
# Perform action:
###

if [ -z "$ACTION" ]
then
	echo "No action chosen!" >&2
	print_help
	exit 1
fi

"$ACTION"
