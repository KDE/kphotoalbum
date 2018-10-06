_checks[check_autostacking]="Feature: Automatic Stacking Of Image Variants"
_context[check_autostacking]="<h2>What this test will do:</h2>
<ul>
<li>This test checks whether the modified image detection works.</li>
<li>After you close this dialog, KPhotoAlbum will be started and it will search for new images.</li>
</ul>
<h2>What you have to do:</h2>
<ol>
<li>Check that the file <tt>grand_canyon_1-edited.jpg</tt> has been stacked with <tt>grand_canyon_1.jpg</tt> and that the tags have been copied.</li>
<li>Check that the file <tt>grand_canyon_1-edited-unrelated.jpg</tt> has not been stacked.</li>
</ol>"
prepare_check_autostacking()
{
	local check_dir="$1"
	echo -e "$BASE_RC" > "$check_dir/kphotoalbumrc"
	convert "$check_dir/db/grand_canyon_1.jpg" -pointsize 60 -draw "gravity center fill red text 0,0 'EDITED - Autostacked'" "$check_dir/db/grand_canyon_1-edited.jpg"
	convert -size 700x460  label:"NOT STACKED" "$check_dir/db/grand_canyon_1-edited-unrelated.jpg"
}
call_check_autostacking()
{
	local check_dir="$1"
	kphotoalbum --db "$check_dir/db/index.xml" --search
}
check_autostacking()
{
	generic_check check_autostacking
}
