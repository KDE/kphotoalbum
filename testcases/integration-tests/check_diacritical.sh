# SPDX-FileCopyrightText: 2019-2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

_checks[check_diacritical]="Compatibility: Diacritical characters in v7 database files."
_context[check_diacritical]="<h2>What this test will do:</h2>
<p><ul>
<li>This test checks whether a version 7 database file with diacritical characters in categories and tags is properly handled. </li>
<li>After you close this dialog, KPhotoAlbum will be started 4 times in a row.</li>
<li>This test case was inspired by <a href=\"https://bugs.kde.org/show_bug.cgi?id=403668\">Bug #403668</a></li>
</ul>
</p>
<h2>What you have to do:</h2>
<ol>
<li>Each time KPhotoAlbum starts, save the database and exit KPhotoAlbum.</li>
</ol>"
check_diacritical()
{
	local check_name="check_diacritical"
	local check_dir="$TEMPDIR/$check_name"
	local data_dir="$mydir/db/diacritical"
	setup_check "$check_dir" || return $result_err_setup
	# not needed in this scenario:
	rm -r "$check_dir/db"

	local automatic
	if [ -n "$NON_INTERACTIVE" ]
	then
		automatic="--save-and-quit"
	else
		kdialog --msgbox "<h1>$check_name</h1>${_context[$check_name]}"
	fi

	for subcheck in compressed uncompressed uncompressed-to-compressed compressed-to-uncompressed
	do
		local subcheck_dir="$check_dir/$subcheck"
		mkdir "$subcheck_dir" || return $result_err_setup

		local add_rc="[General]\nuntaggedCategory=\nuntaggedTag=\n"
		if [[ "$subcheck" == "uncompressed" || "$subcheck" == "compressed-to-uncompressed" ]]
		then
			add_rc="$add_rc\nuseCompressedIndexXML=false\n"
		fi
		echo -e "$add_rc$BASE_RC" > "$subcheck_dir/kphotoalbumrc" || return $result_err_setup

		# set logging rules:
		cp -a "$check_dir/QtProject" "$subcheck_dir"
		# prepare database:
		cp "$data_dir/$subcheck.orig.xml" "$subcheck_dir/index.xml" || return $result_err_setup

		export XDG_CONFIG_HOME="$subcheck_dir"
		kphotoalbum $automatic --db "$subcheck_dir/index.xml" > "$subcheck_dir/log" 2>&1 || return $result_err_crash

		if ! diff -u "$data_dir/$subcheck.result.xml" "$subcheck_dir/index.xml"
		then
			log notice "$check_name: Mismatch in index.xml!"
			return $result_failed
		fi
	done
	return $result_ok
}
