# SPDX-FileCopyrightText: 2019 - 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-2-Clause

_checks[check_old-index.xml]="Compatibility: Old/ancient file formats"
_context[check_old-index.xml]="<h2>What this test will do:</h2>
<p><ul>
<li>This test checks whether old file versions are still correctly read. </li>
<li>After you close this dialog, KPhotoAlbum will be started 2 times in a row.</li>
</ul>
</p>
<h2>What you have to do:</h2>
<ol>
<li>Each time KPhotoAlbum starts, save the database and exit KPhotoAlbum.</li>
</ol>"
check_old-index.xml()
{
	local check_name="check_old-index.xml"
	local check_dir="$TEMPDIR/$check_name"
	local data_dir="$mydir/db/old-index.xml"
	setup_check "$check_dir" || return $result_err_setup
	# not needed in this scenario:
	rm -r "$check_dir/db"

	kdialog --msgbox "<h1>$check_name</h1>${_context[$check_name]}"

	for subcheck in v2.2 v3.0
	do
		local subcheck_dir="$check_dir/$subcheck"
		mkdir "$subcheck_dir" || return $result_err_setup

		local add_rc="[General]\nuntaggedCategory=\nuntaggedTag=\n"
		echo -e "$add_rc$BASE_RC" > "$subcheck_dir/kphotoalbumrc" || return $result_err_setup

		# set logging rules:
		cp -a "$check_dir/QtProject" "$subcheck_dir"
		# prepare database:
		cp "$data_dir/$subcheck.orig.xml" "$subcheck_dir/index.xml" || return $result_err_setup

		export XDG_CONFIG_HOME="$subcheck_dir"
		kphotoalbum --config "$subcheck_dir/kphotoalbumrc" --db "$subcheck_dir/index.xml" > "$subcheck_dir/log" 2>&1 || return $result_err_crash

		if ! diff -u "$data_dir/$subcheck.result.xml" "$subcheck_dir/index.xml"
		then
			log notice "$check_name/$subcheck: Mismatch in index.xml!"
			return $result_failed
		fi
		if ! grep -q '^kphotoalbum.DB: "Standard category names are no longer used since index.xml version 7. Standard categories will be left untranslated from now on."$' "$subcheck_dir/log"
		then
			log notice "$check_name/$subcheck: Missing expected log message!"
			return $result_failed
		fi
	done
	return $result_ok
}
