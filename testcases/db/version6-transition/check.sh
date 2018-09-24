#!/bin/bash -e

mydir=`dirname "$0"`
TESTDIR=`mktemp -d kpa.XXXXXX`
echo "Using temporary directory $TESTDIR" >&2

for _orig in "$mydir"/*.orig.xml
do
	_name=`basename "$_orig"`
	_name="${_name/.orig.xml/}"
	_dir="$TESTDIR/$_name"
	echo "** $_name" >&2

	mkdir "$_dir"
	cp "$_orig" "$_dir/index.xml"
	kphotoalbum --db "$_dir/index.xml"

	if diff -u "$mydir/$_name.result.xml" "$_dir/index.xml"
	then
		echo "   SUCCESS" >&2
	else
		echo "   FAILED" >&2
	fi
done

echo "Please remember to remove $TESTDIR" >&2
