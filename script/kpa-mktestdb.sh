#!/bin/bash
###
PROGNAME=kpa-mktestdb
VERSION="0.1"
COPYRIGHT="Copyright 2014 Johannes Zarl <johannes at zarl dot at>"
LICENSE="LGPL-3"
###
LOCALPREFIX=`kde4-config --localprefix`
# default locations:
KPARC=$LOCALPREFIX/share/config/kphotoalbumrc
###
TEMPLATE=
DESTINATION=
NO_ACT=
VERBOSE=

get_config_value()
{
	if [ -r "$KPARC" ]
	then
		sed -n 's/#.*// ; s/'$1'=\(.*\)/\1/p' "$KPARC"
	else
		echo "$KPARC does not exist!" >&2
		return 1
	fi
}

print_version()
{
	echo "$PROGNAME $VERSION"
	echo "$COPYRIGHT"
	echo "This work is licensed under: $LICENSE"
}

print_help() {
	echo "Usage: $0 [-n] DESTINATION" >&2
	echo "" >&2
	echo "Create a KPhotoAlbum test database directory from a template." >&2
	echo "index.xml, thumbnails, and exif databases are copied," >&2
	echo "everything else is symlinked." >&2
	echo "" >&2
	echo "Options:" >&2
	echo "-n|--no-act    Only print what would be done, don't change files." >&2
	echo "-v|--verbose   Print commands as they are executed." >&2
	echo "--version      Print version information" >&2
	echo "--template     Specify an alternative template directory." >&2
	echo "               (default: $TEMPLATE)" >&2
	echo "" >&2
}

INDEXFILE=`get_config_value configfile`
if [ -f "$INDEXFILE" ]
then
	TEMPLATE=`dirname "$INDEXFILE"`
fi

### Parse commandline:
TEMP=`getopt -o hnv --long help,no-act,template:,version,verbose \
     -n "$PROGNAME" -- "$@"`

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

while true ; do
	case "$1" in
		-h|--help) print_help ; exit ;;
		--version) print_version ; exit ;;
		-v|--verbose) VERBOSE=1 ; shift ;;
		-n|--no-act) NO_ACT=1 ; shift ;;
		-t|--template) TEMPLATE="$2" ; shift 2 ;;
		--) shift ; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done

if [ "$#" -eq 0 ]
then
	echo "Missing destination directory!" >&2
	exit 1
fi

if [ "$#" -gt 1 ]
then
	echo "Unknown extra parameters: $@" >&2
	exit 1
fi

DESTINATION="$1"

if [ -e "$DESTINATION" ]
then
	echo "Destination directory exists. Bailing out..." >&2
	exit 2
fi

if [ ! -d "$TEMPLATE" ]
then
	echo "Template does not exist. Bailing out..." >&2
	exit 2
fi

if [ -n "$NO_ACT" ]
then
	act() {
		echo "NOT executing: $*"
	}
else
	act() {
		if [ -n "$VERBOSE" ]
		then
			echo "Executing: $*"
		fi
		"$@"
	}
fi

###### Do the work:

REQUIRED="index.xml"
OPTIONAL="exif-info.db recognition.db .thumbnails .videoThumbnails"

# check requirements:
if [ ! -d "$TEMPLATE" ]
then
	echo "Template directory does not exist!" >&2
	exit 3
fi
for f in $REQUIRED
do
	f="$TEMPLATE/$f"
	if [ ! -e "$f" ]
	then
		echo "Missing file: $f" >&2
		exit 3
	fi
done

if ! act mkdir "$DESTINATION"
then
	echo "Could not create destination directory!"
	exit 3
fi

# copy files:
files=`ls -A1 "$TEMPLATE"`
for f in $files
do
	if echo "$REQUIRED $OPTIONAL" |grep -q "\<$f\>"
	then
		act cp "$TEMPLATE/$f" "$DESTINATION"
	else
		act ln -s "$TEMPLATE/$f" "$DESTINATION"
	fi
done
