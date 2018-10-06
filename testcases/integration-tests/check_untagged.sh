_checks[check_untagged]="Feature: Mark As Untagged"
_context[check_untagged]="<h2>What this test will do:</h2>
<ul>
<li>This test checks if new images are properly tagged with the <em>untagged</em> tag after import.</li>
<li>After you close this dialog, KPhotoAlbum will be started and it will search for new images.</li>
</ul>
<h2>What you have to do:</h2>
<ol>
<li>Go to <em>Untagged Images</em> and see if the image of a rose shows up.</li>
<li>Check if the image of a rose has the *untagged* tag set.</li>
</ol>"
prepare_check_untagged()
{
	local check_dir="$1"
	echo -e "[General]\nuntaggedImagesTagVisible=true\nuntaggedCategory=Events\nuntaggedTag=untagged\n$BASE_RC" > "$check_dir/kphotoalbumrc"
	convert -size 700x460  magick:rose "$check_dir/db/rose.jpg"
}
call_check_untagged()
{
	local check_dir="$1"
	kphotoalbum --db "$check_dir/db/index.xml" --search
}
check_untagged()
{
	generic_check check_untagged
}
