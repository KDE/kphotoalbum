#!/bin/bash -e
###
# Script for generating a proper export of the source tree, generating
# the VERSION file.
###

_wd="`dirname "$0"`/.."

# work from base directory of the repository
cd $_wd

# update version file:
cmake -P cmake/modules/UpdateVersion.cmake
if grep -q '[-]dirty' version.h
then
	echo "It seems that you are exporting a source tree with local modifications." >&2
	echo "If you continued, you would get a tarball without your local modifications," >&2
	echo "but with a version.h that is marked \"dirty\"." >&2
	echo "Please commit your changes and try again!" >&2
	exit 1
fi

_version=`git describe`
_prefix=kphotoalbum-${_version}
_archive=$_prefix.tar
echo "Creating $_archive for current branch ..." >&2
git archive --format tar --prefix="$_prefix/" -o "$_archive" "HEAD"

echo "Adding version.h ..." >&2
tar --append -f "$_archive" --transform 's!^!'"$_prefix/"'!' "version.h"

echo "Compressing archive ..." >&2
bzip2 "$_archive"
