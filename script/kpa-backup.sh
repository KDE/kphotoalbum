#!/bin/bash
# Author: Johannes Zarl <isilmendil@gmx.at>
# default locations:
KPARC=$HOME/.kde/share/config/kphotoalbumrc
KPAUIRC=$HOME/.kde/share/apps/kphotoalbum/kphotoalbumui.rc
BACKEND=
INDEXFILE=
KPA_FOLDER=
BACKUP_LOCATION=~/kpa-backup
ACTION=
ADD_FILES_RELATIVE="exif-info.db layout.dat"

###
# Helper functions:
###

get_backend_type()
{
	sed -n 's/#.*// ; s/backend=\(.*\)/\1/p' "$KPARC"
}

get_index_location()
{
	sed -n 's/#.*// ; s/configfile=\(.*\)/\1/p' "$KPARC"
}

print_help()
{
	echo "Usage: $0 -b|--backup [-d|--directory BACKUP_LOCATION]" >&2
	echo "       $0 -r|--restore [-d|--directory BACKUP_LOCATION]" >&2
	echo "" >&2
	echo "Create or restore a backup of your essential KPhotoalbum files." >&2
	echo "Note: your actual image-files are not backed up!" >&2
	echo "" >&2
}

do_backup()
{
	if [ ! -d "$BACKUP_LOCATION" ]
	then
		echo "Backup location ($BACKUP_LOCATION) is not a directory, creating it." >&2
		mkdir "$BACKUP_LOCATION"
	fi
	BACKUP_LOCATION_WDATE="$BACKUP_LOCATION"/"`date +%Y%m%d-%H%M%S`"
	mkdir "$BACKUP_LOCATION_WDATE"
	rm "$BACKUP_LOCATION"/latest
	ln -s "$BACKUP_LOCATION_WDATE" "$BACKUP_LOCATION"/latest

	echo "Backing up essential files..."
	for f in "$KPARC" "$KPAUIRC" "$INDEXFILE"
	do
		tar -Pcvz -f "$BACKUP_LOCATION_WDATE"/`basename "$f"`.tgz "$f"
	done
	echo "Backing up additional files..."
	for f in $ADD_FILES_RELATIVE
	do
		[ -f "$KPA_FOLDER/$f" ] && tar -Pcvz -f "$BACKUP_LOCATION_WDATE"/`basename "$f"`.tgz "$KPA_FOLDER/$f"
	done
}

do_restore()
{
	echo "Restoring essential files..."
	for f in "$KPARC" "$KPAUIRC" "$INDEXFILE"
	do
		tar -Pwxvz -f "$BACKUP_LOCATION/latest/`basename "$f"`".tgz
	done
	echo "Restoring additional files..."
	for f in $ADD_FILES_RELATIVE
	do
		[ -f "$BACKUP_LOCATION/$f" ] && tar -Pwxvz -f "$BACKUP_LOCATION/latest/`basename "$f"`".tgz
	done
}

###
# Parse commandline:
###

TEMP=`getopt -o hbrd: --long help,backup,restore,directory: \
     -n 'kpa-backup' -- "$@"`

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

while true ; do
	case "$1" in
		-h|--help) print_help ; exit ;;
		-b|--backup) ACTION=backup ; shift ;;
		-r|--restore) ACTION=restore ; shift ;; 
		-d|--directory) BACKUP_LOCATION="$2" ; shift 2 ;;
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
# Query file locations from RC files & check parameter sanity:
###

if [ ! -r "$KPARC" ]
then
	echo "RC-file ($KPARC) not readable!" >&2
	exit 1
fi
if [ ! -r "$KPAUIRC" ]
then
	echo "User-interface RC-file ($KPAUIRC) not readable!" >&2
	exit 1
fi

BACKEND=`get_backend_type`
case "$BACKEND" in
	xml)
		echo "KPhotoalbum uses XML backend..."
		INDEXFILE=`get_index_location`
		if [ "$ACTION" == "backup" ] && [ ! -r "$INDEXFILE" ]
		then
			echo "Kphotoalbum XML database file ($INDEXFILE) not readable!" >&2
			exit 1
		fi
		KPA_FOLDER=`dirname "$INDEXFILE"`
		if [ ! -d "$KPA_FOLDER" ]
		then
			echo "Kphotoalbum image directory ("$KPA_FOLDER") does not exist!" >&2
			exit 1
		fi
		;;
	*)
		echo "KPhotoalbum uses backend \`$BACKEND'..."
		echo "This backend is not currently supported!"
		exit 1
		;;
esac

###
# Perform action:
###

case "$ACTION" in
	backup) do_backup ;;
	restore) do_restore ;;
	*) 
		echo "No action chosen!" >&2
		print_help
		exit 1
		;;
esac
		
