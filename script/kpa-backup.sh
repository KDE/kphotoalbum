#!/bin/bash
# Author: Johannes Zarl <isilmendil@gmx.at>
# default locations:
KPARC=$HOME/.kde/share/config/kphotoalbumrc
KPAUIRC=$HOME/.kde/share/apps/kphotoalbum/kphotoalbumui.rc
BACKUP_LOCATION=~/kpa-backup
ACTION=
ADD_FILES_RELATIVE="exif-info.db layout.dat"
KEEP_NUM=5
NO_ACT=

###
# Helper functions:
###

get_config_value()
{
	sed -n 's/#.*// ; s/'$1'=\(.*\)/\1/p' "$KPARC"
}

print_help()
{
	echo "Usage: $0 -b|--backup [-d|--directory BACKUP_LOCATION]" >&2
	echo "       $0 -r|--restore [-d|--directory BACKUP_LOCATION]" >&2
	echo "       $0 -l|--list [-d|--directory BACKUP_LOCATION]" >&2
	echo "       $0 -p|--purge [--keep NUM]" >&2
	echo "" >&2
	echo "Create or restore a backup of your essential KPhotoalbum files." >&2
	echo "Note: your actual image-files are not backed up!" >&2
	echo "" >&2
	echo "-d|--directory BACKUP_LOCATION   Use the specified path as backup location" >&2
	echo "                                 [default: $BACKUP_LOCATION]" >&2
	echo "--keep NUM                       Keep the latest NUM backups" >&2
	echo "                                 [default: $KEEP_NUM]" >&2
	echo "-n|--no-act                      Do not take any action." >&2
	echo "" >&2
}

untar_if_changed()
# untar the given single-file-tarball if the destination file has changed
# use first parameter -p to print when the file has changed, but not untar it.
{
	local printonly=false
	[ -n "$NO_ACT" ] && printonly=true
	local diffredir="/dev/stdout"
	local tarfile
	for param
	do
		case "$param" in
			-p)
				printonly=true
				;;
			-s) #silent
				diffredir="/dev/null"
				;;
			*)
				tarfile="$param"
				;;
		esac
	done
	[ -f "$tarfile" ] || return 1
	local dstfile=`tar -Ptz -f "$tarfile"`
	if tar -PxzO -f "$tarfile" | diff -u - "$dstfile" >$diffredir
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
	local BACKEND=
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
	# KPA gets the image directory from the configfile entry
	INDEXFILE=`get_config_value configfile`
	KPA_FOLDER=`dirname "$INDEXFILE"`
	if [ ! -d "$KPA_FOLDER" ]
	then
		echo "Kphotoalbum image directory ($KPA_FOLDER) does not exist!" >&2
		exit 1
	fi

	BACKEND=`get_config_value backend`
	case "$BACKEND" in
		xml)
			echo "KPhotoalbum uses XML backend..."
			if [ ! -r "$INDEXFILE" ]
			then
				echo "Kphotoalbum XML database file ($INDEXFILE) not readable!" >&2
				exit 1
			fi
			;;
		*)
			echo "KPhotoalbum uses backend \`$BACKEND'..." >&2
			echo "This backend is not currently supported!" >&2
			exit 1
			;;
	esac

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
	echo "Restoring essential files..."
	for f in "$BACKUP_LOCATION/latest"/*.tgz
	do
		# untar_if_changed honors NO_ACT:
		untar_if_changed "$f"
	done
}

do_list()
{
	local LATEST=`readlink "$BACKUP_LOCATION/latest"`
	LATEST=`basename "$LATEST"`
	echo "$BACKUP_LOCATION:"
	for d in "$BACKUP_LOCATION"/*
	do
		if [ -d "$d" ]
		then
			[ -L "$d" ] && continue
			backup_name=`basename "$d"`
			echo -n " -$backup_name" | sed 's/\(....\)\(..\)\(..\)-\(..\)\(..\)\(..\)/\0 [\1-\2-\3 \4:\5:\6]/'
			if [ "$backup_name" = "$LATEST" ]
			then
				echo -n " (*latest*)"
			fi
			echo
			for f in "$d"/*.tgz
			do
				echo -n "  |-"
				untar_if_changed -p -s "$f"
			done
			echo
		fi
	done
}

do_purge()
{
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

TEMP=`getopt -o hbrlpnd: --long help,backup,restore,list,purge,no-act,directory:,keep: \
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
		-p|--purge) ACTION=do_purge ; shift ;;
		-n|--no-act) NO_ACT=1 ; shift ;;
		-d|--directory) BACKUP_LOCATION="$2" ; shift 2 ;;
		--keep) KEEP_NUM="$2" ; shift 2 ;;
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
