# SPDX-FileCopyrightText: 2018 - 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

_checks[check_skipRawIfOtherMatches]="Feature: Do not read RAW files if a matching JPEG/TIFF file exists"
_context[check_skipRawIfOtherMatches]="<h2>What this test will do:</h2>
<ul>
<li>This test checks whether RAW files are ignored if a matching jpeg or tiff file exists.</li>
<li>The test checks different raw file extensions to rule out accidental correctness through alphabetical order.</li>
<li>After you close this dialog, KPhotoAlbum will be started and it will search for new images.</li>
</ul>
<h2>How you can check manually</h2>
<ol>
<li>Go to <em>Untagged Images</em> and check that four new images show up:</li>
  <ul>
  <li>test-dng-jpg.jpg</li>
  <li>test-raw-jpg.jpg</li>
  <li>test-dng-tiff.tiff</li>
  <li>test-raw-tiff.tiff</li>
  </ul>
<li>Check that no other new images show up.
</ol>
Note that all of these files are jpeg/tiff files, because ImageMagick cannot write raw files and because the decoding part is not actually what is tested here.
<h2>What you have to do:</h2>
<ol>
<li>When KPhotoAlbum starts, save the database and exit KPhotoAlbum.</li>
</ol>"
_check_db_file[check_skipRawIfOtherMatches]=integration-tests/check_skipRawIfOtherMatches.result.xml

create_image()
# create_image FILENAME LABEL
{
	local fn="$1"
	local lbl="$2"
	convert -size 700x460 label:"$lbl" "$fn"
	exiv2 -M"set Exif.Photo.DateTimeOriginal '2000-01-01 00:00:00'" "$fn"
}

prepare_check_skipRawIfOtherMatches()
{
	local check_dir="$1"
	echo -e "[ExifImport]\nskipRawIfOtherMatches=true\n$BASE_RC" > "$check_dir/kphotoalbumrc"

	for ext1 in dng raw
	do
		for ext2 in jpg tiff
		do
			# instead of generating the files on the fly, copy a pre-generated version instead:
			#create_image "$mydir/integration-tests/check_skipRawIfOtherMatches/$ext1-$ext2.$ext2" "$ext1-$ext2"
			#create_image "$mydir/integration-tests/check_skipRawIfOtherMatches/$ext1-$ext2.$ext2.jpg" "$ext1-$ext2 (to be ignored)"
			#mv "$mydir/integration-tests/check_skipRawIfOtherMatches/$ext1-$ext2.$ext2.jpg" "$mydir/integration-tests/check_skipRawIfOtherMatches/$ext1-$ext2.$ext1"
			# ...this way, we are guaranteed to get the same md5 sum every time:
			cp -a "$mydir/integration-tests/check_skipRawIfOtherMatches/$ext1-$ext2.$ext1" "$check_dir/db/"
			cp -a "$mydir/integration-tests/check_skipRawIfOtherMatches/$ext1-$ext2.$ext2" "$check_dir/db/"
		done
	done
}
call_check_skipRawIfOtherMatches()
{
	local check_dir="$1"
	local automatic
	if [ -n "$NON_INTERACTIVE" ]
	then
		automatic="--save-and-quit"
	fi
	kphotoalbum $automatic --config "$check_dir/kphotoalbumrc" --db "$check_dir/db/index.xml" --search
}
check_skipRawIfOtherMatches()
{
	generic_check check_skipRawIfOtherMatches
}
