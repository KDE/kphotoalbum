# SPDX-FileCopyrightText: 2018 - 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

_checks[check_stripexif]="Feature: Strip Camera Generated Default Descriptions"
_context[check_stripexif]="<h2>What this test will do:</h2>
<ul>
<li>This test checks whether Exif comments are properly imported into the description field of the database.</li>
<li>The filter for stripping camera generated default descriptions is also tested.</li>
<li>After you close this dialog, KPhotoAlbum will be started and it will search for new images.</li>
</ul>
<h2>How you can check manually</h2>
<ol>
<li>Go to <em>Untagged Images</em> and check that two new images show up.</li>
<li>Check if the image with text <em>DEFAULT_DESCRIPTION</em> has an empty description field.</li>
<li>Check if the image with text <em>NO DEFAULT_DESCRIPTION</em> has the description \"NO DEFAULT DESCRIPTION\".</li>
</ol>
<h2>What you have to do:</h2>
<ol>
<li>When KPhotoAlbum starts, save the database and exit KPhotoAlbum.</li>
</ol>"
_check_db_file[check_stripexif]=integration-tests/check_stripexif.result.xml

check_stripexif_exifhelper()
# generate an image with exiv description
{
	convert -size 700x460  label:"$1" "$check_dir/db/$1.jpg"
	exiv2 -M"set Exif.Image.ImageDescription $1" "$check_dir/db/$1.jpg"
	exiv2 -M"set Exif.Photo.DateTimeOriginal '2000-01-01 00:00:00'" "$check_dir/db/$1.jpg"
}

prepare_check_stripexif()
{
	local check_dir="$1"
	# useEXIFComments is set by default
	# stripEXIFComments contains several descriptions, line by line
	# make sure there's more than one line so that escaping is also checked:
	echo -e "[General]\nstripEXIFComments=true\ncommentsToStrip=SOME DEFAULT-,-DEFAULT_DESCRIPTION-,-OTHER-,-\n$BASE_RC" > "$check_dir/kphotoalbumrc"

	# instead of generating the files on the fly, copy a pre-generated version instead:
	#check_stripexif_exifhelper "DEFAULT_DESCRIPTION"
	#check_stripexif_exifhelper "NO DEFAULT_DESCRIPTION"
	# ...this way, we are guaranteed to get the same md5 sum every time:
	cp -a "$mydir/integration-tests/check_stripexif/DEFAULT_DESCRIPTION.jpg" "$check_dir/db/"
	cp -a "$mydir/integration-tests/check_stripexif/NO DEFAULT_DESCRIPTION.jpg" "$check_dir/db/"
}
call_check_stripexif()
{
	local check_dir="$1"
	kphotoalbum --config "$check_dir/kphotoalbumrc" --db "$check_dir/db/index.xml" --search
}
check_stripexif()
{
	generic_check check_stripexif
}
