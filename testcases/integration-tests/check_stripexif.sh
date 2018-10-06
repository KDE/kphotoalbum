_checks[check_stripexif]="Feature: Strip Camera Generated Default Descriptions"
_context[check_stripexif]="<h2>What this test will do:</h2>
<ul>
<li>This test checks whether Exif comments are properly imported into the description field of the database.</li>
<li>The filter for stripping camera generated default descriptions is also tested.</li>
<li>After you close this dialog, KPhotoAlbum will be started and it will search for new images.</li>
</ul>
<h2>What you have to do:</h2>
<ol>
<li>Go to <em>Untagged Images</em> and check that two new images show up.</li>
<li>Check if the image with text <em>DEFAULT_DESCRIPTION</em> has an empty description field.</li>
<li>Check if the image with text <em>NO DEFAULT_DESCRIPTION</em> has the description \"NO DEFAULT DESCRIPTION\".</li>
</ol>"

check_stripexif_exifhelper()
# generate an image with exiv description
{
	convert -size 700x460  label:"$1" "$check_dir/db/$1.jpg"
	exiv2 -M"set Exif.Image.ImageDescription $1" "$check_dir/db/$1.jpg"
}

prepare_check_stripexif()
{
	local check_dir="$1"
	# useEXIFComments is set by default
	# stripEXIFComments contains several descriptions, line by line
	# make sure there's more than one line so that escaping is also checked:
	echo -e "[General]\nstripEXIFComments=true\ncommentsToStrip=SOME DEFAULT-,-DEFAULT_DESCRIPTION-,-OTHER-,-\n$BASE_RC" > "$check_dir/kphotoalbumrc"
	check_stripexif_exifhelper "DEFAULT_DESCRIPTION"
	check_stripexif_exifhelper "NO DEFAULT_DESCRIPTION"
}
call_check_stripexif()
{
	local check_dir="$1"
	kphotoalbum --db "$check_dir/db/index.xml" --search
}
check_stripexif()
{
	generic_check check_stripexif
}
